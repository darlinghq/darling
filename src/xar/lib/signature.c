/*
 * Copyright (c) 2006 Apple Computer, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Apple nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * 6-July-2006
 * DRI: Christopher Ryan <ryanc@apple.com>
*/
 
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlreader.h>
#include <inttypes.h>  /* for PRIu64 */
#include <unistd.h>

#include "xar.h"
#include "archive.h"
#include "b64.h"
#include "signature.h"

struct _signature_copy_context{
	void *buffer;
	size_t length;
	off_t offset;
};

#ifdef __APPLE__
xar_signature_t xar_signature_new_internal(xar_t x, int is_extended, const char *type, int32_t length, xar_signer_callback callback, void *callback_context)
#else
xar_signature_t xar_signature_new(xar_t x,const char *type, int32_t length, xar_signer_callback callback, void *callback_context)
#endif
{
	xar_signature_t ret;
		
	if( XAR(x)->files ){
		xar_err_new(x);
		xar_err_set_string(x, "Signatures must be added before files are added");
		xar_err_callback(x, XAR_SEVERITY_WARNING, XAR_ERR_ARCHIVE_CREATION);
		return NULL;
	}
		
	ret = malloc(sizeof(struct __xar_signature_t));
	if( ! ret )
		return NULL;
	
	memset(XAR_SIGNATURE(ret), 0, sizeof(struct __xar_signature_t));
	XAR_SIGNATURE(ret)->type = strdup(type);
	XAR_SIGNATURE(ret)->len = length;
	XAR_SIGNATURE(ret)->offset = XAR(x)->heap_offset;
	XAR_SIGNATURE(ret)->signer_callback = callback;
	XAR_SIGNATURE(ret)->callback_context = callback_context;
#ifdef __APPLE__
    XAR_SIGNATURE(ret)->is_extended = is_extended;
#endif
	
	/* Pre allocate space in the heap */
	XAR(x)->heap_offset += length;
	XAR(x)->heap_len += length;

	// Put the new on at the end of the list
	if( XAR_SIGNATURE(XAR(x)->signatures) ) {
		struct __xar_signature_t *sig;
		
		// (Apple) Find the end of the signatures list
		// The open source code seems to smash list items 1 through n when the head of this list is already defined
		// despite the fact that the xar signatures are implemented as a list.  This is an Apple xar 1.4 edit
		// to allow that list to grow beyond 2
		for( sig = XAR_SIGNATURE(XAR(x)->signatures); sig->next; sig = sig->next);
		
		sig->next = XAR_SIGNATURE(ret);
	} else
		XAR(x)->signatures = ret;

	XAR_SIGNATURE(ret)->x = x;
	
	return ret;
	
}

#ifdef __APPLE__
xar_signature_t xar_signature_new(xar_t x,const char *type, int32_t length, xar_signer_callback callback, void *callback_context)
{
    return xar_signature_new_internal(x, 0, type, length, callback, callback_context);
}
#endif

xar_signature_t xar_signature_new_extended(xar_t x,const char *type, int32_t length, xar_signer_callback callback, void *callback_context)
{
#ifdef __APPLE__
    return xar_signature_new_internal(x, 1, type, length, callback, callback_context);
#else
    return xar_signature_new(x, type, length, callback, callback_context);
#endif
}

const char *xar_signature_type(xar_signature_t s)
{
	if( !s )
		return NULL;
		
	return XAR_SIGNATURE(s)->type;
}

/* Add x509 cert data to this signature */
int32_t xar_signature_add_x509certificate(xar_signature_t sig, const uint8_t *cert_data, uint32_t cert_len )
{
	struct __xar_x509cert_t *newcert;
	
	if( !sig )
		return -1;

	newcert = malloc(sizeof(struct __xar_x509cert_t));
	memset(newcert, 0, sizeof(struct __xar_x509cert_t));
	newcert->content = malloc(sizeof(const char)*cert_len);
	memcpy(newcert->content,cert_data,cert_len);
	newcert->len = cert_len;
	
	if( XAR_SIGNATURE(sig)->x509certs ){
		struct __xar_x509cert_t *cert;
		
		// Find the end of the linked list
		for( cert = XAR_SIGNATURE(sig)->x509certs; cert->next; cert = cert->next );
		
		cert->next = newcert;
	}else{
		XAR_SIGNATURE(sig)->x509certs = newcert;		
	}
	
	XAR_SIGNATURE(sig)->x509cert_count++;
	
	return 0;
}

int32_t xar_signature_get_x509certificate_count(xar_signature_t sig)
{
	if( !sig ){
		return 0;
	}
	
	return XAR_SIGNATURE(sig)->x509cert_count;
}

int32_t xar_signature_get_x509certificate_data(xar_signature_t sig, int32_t index, const uint8_t **cert_data, uint32_t *cert_len)
{
	struct __xar_x509cert_t *cert;
	int i = 0;
	
	/* If there are no certs to return, return immediatly */
	if( !XAR_SIGNATURE(sig)->x509cert_count ){
		if( cert_data )
			*cert_data = 0;
		
		return -1;
	}
	
	
	/* Loop through the certs, copying each one's string ptr to the array */
	for( cert = XAR_SIGNATURE(sig)->x509certs; cert; cert = cert->next ){
		if( i == index ){
			if( cert_data )
				*cert_data = cert->content;
			
			if( cert_len )
				*cert_len = cert->len;
			break;
		}
		i++;
	}
	
	/* If the cert iterator is still valid, we did not find the proper index */
	if( !cert ){
		return -1;
	}
	
	return 0;
}

xar_signature_t xar_signature_first(xar_t x)
{
	return XAR(x)->signatures;
}

xar_signature_t xar_signature_next(xar_signature_t s)
{
	return XAR_SIGNATURE(s)->next;
}

int32_t _xar_signature_read_from_heap(xar_t x ,off_t offset,size_t length,uint8_t *data)
{
	off_t seek_off = XAR(x)->toc_count + sizeof(xar_header_t) + offset;
	int r = 0;
	
	r = lseek(XAR(x)->fd, seek_off, SEEK_SET);
	
	/* can't seek to the proper location */
	if( -1 == r ){
		xar_err_new(x);
		xar_err_set_string(x, "Unable to seek");
		xar_err_callback(x, XAR_SEVERITY_NONFATAL, XAR_ERR_ARCHIVE_EXTRACTION);		
		return -1;
	}
		
	r = read(XAR(x)->fd,data,length);
	
	if( r != length ){
		xar_err_new(x);
		xar_err_set_string(x, "Unable to read");
		xar_err_callback(x, XAR_SEVERITY_NONFATAL, XAR_ERR_ARCHIVE_EXTRACTION);		
		return -1;		
	}
	
	return 0;
}

/* This method retrieves the signed data for this segment as well as the data the signed data is signing */
uint8_t xar_signature_copy_signed_data(xar_signature_t sig, uint8_t **data, uint32_t *length, uint8_t **signed_data, uint32_t *signed_length, off_t *signed_offset)
{
	uint32_t offset = 0;
	xar_t x = NULL;
	const char	*value;
	
	// xar 1.6 fails this method if any of data, length, signed_data, signed_length are NULL
	// within OS X we use this method to get combinations of signature, signed data, or signed_offset,
	// so this method checks and sets these out values independently
	
	if( !sig )
		return -1;
	
	x = XAR_SIGNATURE(sig)->x;
	
	
	/* Get the checksum, to be used for signing.  If we support multiple checksums
		in the future, all checksums should be retrieved						*/
	if(length) {
		if(0 == xar_prop_get( XAR_FILE(x) , "checksum/size", &value)){
			*length  = strtoull( value, (char **)NULL, 10);
		}

		if(0 == xar_prop_get( XAR_FILE(x) , "checksum/offset", &value)){
			offset  = strtoull( value, (char **)NULL, 10);
		}
	
		if(data) {
			*data = malloc(sizeof(char)*(*length));

			_xar_signature_read_from_heap(x, offset, *length, *data);
		}
	}
	
	/* read signature data */
	offset  = XAR_SIGNATURE(sig)->offset;
	
	// This parameter is an Apple edit
	// This out value is ignored OS X in SL, but this method prototype is included in the 10.5 SDK
	if( signed_offset )
		*signed_offset = offset;
	
	if(signed_length) {
		
		*signed_length  = XAR_SIGNATURE(sig)->len;
		
		if(signed_data) {
			*signed_data = malloc(sizeof(char)*(*signed_length));
	
			_xar_signature_read_from_heap(x, offset, *signed_length, *signed_data);
		}
	}
				
	return 0;
}

xar_signature_t xar_signature_unserialize(xar_t x, xmlTextReaderPtr reader)
{
	struct __xar_signature_t *ret = NULL;
#ifdef __APPLE__
    const xmlChar *sig_type = NULL;
#endif
	const xmlChar *value = NULL;
	const xmlChar *name = NULL;
	int type;
	size_t outputLength = 0;
	
	ret = malloc(sizeof(struct __xar_signature_t));
	
	if( ! ret )
		return NULL;
	
	memset(ret, 0, sizeof(struct __xar_signature_t));
	
	ret->x = x;
	
	/* Read One Signature Element */
#ifdef __APPLE__
    sig_type = xmlTextReaderConstLocalName(reader);
    if(strcmp((const char*)sig_type, "x-signature") == 0) {
        ret->is_extended = 1;
    }
#endif
		
	/* Retrieve the type attr */
	value = xmlTextReaderGetAttribute(reader, (const xmlChar*)"style");

	if( value ){
		ret->type = strdup((const char *)value);
	}
	
	/* Look for the rest of the child elements */
	while( xmlTextReaderRead(reader) == 1 ) {
		type = xmlTextReaderNodeType(reader);
		name = xmlTextReaderConstLocalName(reader);
	
		if (value) {
			xmlFree((void*)value);
			value = NULL;
		}
		
		if( type == XML_READER_TYPE_ELEMENT ) {
			if(strcmp((const char*)name, "size") == 0) {
				while( xmlTextReaderRead(reader) == 1 ) {
					type = xmlTextReaderNodeType(reader);
					if( type == XML_READER_TYPE_TEXT ) {
						
						value = xmlTextReaderConstValue(reader);				
						ret->len = strtoull( (const char *)value, (char **)NULL, 10);
						value = NULL; /* We don't want to accidentally release this const. */	
					}else if( type == XML_READER_TYPE_END_ELEMENT ) {
						break;
					}
				}
			} else if( strcmp((const char*)name, "offset") == 0 ){
				while( xmlTextReaderRead(reader) == 1 ) {
					type = xmlTextReaderNodeType(reader);
					if( type == XML_READER_TYPE_TEXT ) {
						
						value = xmlTextReaderConstValue(reader);				
						ret->offset = strtoull( (const char *)value, (char **)NULL, 10);
						value = NULL; /* We don't want to accidentally release this const. */
					}else if( type == XML_READER_TYPE_END_ELEMENT ) {
						break;
					}
				}
			} else if( strcmp((const char*)name, "KeyInfo") == 0 ){
				while( xmlTextReaderRead(reader) == 1 ) {
					type = xmlTextReaderNodeType(reader);
					name = xmlTextReaderConstLocalName(reader);
					
					if( type == XML_READER_TYPE_ELEMENT ) {
						if(strcmp((const char*)name, "X509Data") == 0) {
							while( xmlTextReaderRead(reader) == 1 ) {
								type = xmlTextReaderNodeType(reader);
								name = xmlTextReaderConstLocalName(reader);
								
								if( type == XML_READER_TYPE_ELEMENT ) {
									if(strcmp((const char*)name, "X509Certificate") == 0) {
										while( xmlTextReaderRead(reader) == 1 ) {
											type = xmlTextReaderNodeType(reader);
											if( type == XML_READER_TYPE_TEXT ) {
												unsigned char *sig_data = NULL;
												
												value = xmlTextReaderConstValue(reader);
												
												/* this method allocates the resulting data */
												sig_data = xar_from_base64(value, strlen((const char *)value), &outputLength);
												
												if(sig_data){
                                                    /* for convience we just use the same method, which means multiple allocations */
                                                    xar_signature_add_x509certificate(ret, sig_data, outputLength);
                                                    free(sig_data);
                                                }else{
                                                    ret = NULL;
                                                }
												
												value = NULL; /* We don't want to accidentally release this const. */
												//break;
											}else if( type == XML_READER_TYPE_END_ELEMENT ) {
												break;
											}
										}										
									}
								}else if( type == XML_READER_TYPE_END_ELEMENT ) {
									break;
								}
							}							
						}
					}else if( type == XML_READER_TYPE_END_ELEMENT ) {
						break;
					}
					
				}
			}			
		}else if( type == XML_READER_TYPE_TEXT ) {

			value = xmlTextReaderConstValue(reader);
			value = NULL; /* We don't want to accidentally release this const. */

			break;
		}else if( type == XML_READER_TYPE_END_ELEMENT ) {
			break;
		}
	}
	
	if (value) {
		xmlFree((void*)value);
		value = NULL;
	}
	
	return ret;
}

/*
 <signature style="SHA1withRSA">
	<offset>20</offset>
	<size>256</size>
	<KeyInfo xmlns="http://www.w3.org/2000/09/xmldsig#">
		<X509Data>
			<X509Certificate>MIICXTCCA...base64...</X509Certificate>
		</X509Data>
	</KeyInfo>
 </signature>
 */
/* This function will serialize an entire list of signatures */
int32_t xar_signature_serialize(xar_signature_t sig, xmlTextWriterPtr writer)
{	
	if( !sig ) return 0;

	/* <signature type='EncryptionAlgorithm'> */
#ifdef __APPLE__
    const char* element_name = XAR_SIGNATURE(sig)->is_extended ? "x-signature" : "signature";
	xmlTextWriterStartElementNS( writer, NULL, BAD_CAST(element_name), NULL);
#else
	xmlTextWriterStartElementNS( writer, NULL, BAD_CAST("signature"), NULL);		
#endif
	
	/* write out the style */
	xmlTextWriterWriteAttribute(writer, BAD_CAST("style"), BAD_CAST(XAR_SIGNATURE(sig)->type));

	/* <offset> */
	xmlTextWriterStartElementNS( writer, NULL, BAD_CAST("offset"), NULL);		
	xmlTextWriterWriteFormatString(writer, "%"PRIu64, (uint64_t)(XAR_SIGNATURE(sig)->offset));
	xmlTextWriterEndElement(writer);	

	/* <size> */
	xmlTextWriterStartElementNS( writer, NULL, BAD_CAST("size"), NULL);
	xmlTextWriterWriteFormatString(writer, "%d", (XAR_SIGNATURE(sig)->len));
	xmlTextWriterEndElement(writer);	

	/* <KeyInfo xmlns="http://www.w3.org/2000/09/xmldsig#"> */
	xmlTextWriterStartElementNS( writer, NULL, BAD_CAST("KeyInfo"), NULL);
	xmlTextWriterWriteAttribute(writer, BAD_CAST("xmlns"), BAD_CAST("http://www.w3.org/2000/09/xmldsig#"));
	
	/* If we have x509 certs, write them out */
	if( XAR_SIGNATURE(sig)->x509certs ){
		struct __xar_x509cert_t *cert;
		
		/* <X509Data> */
		xmlTextWriterStartElementNS( writer, NULL, BAD_CAST("X509Data"), NULL);
			
		/* Loop through the certs, copying each one's string ptr to the array */
		for( cert = XAR_SIGNATURE(sig)->x509certs; cert; cert = cert->next ){
			xmlTextWriterStartElementNS( writer, NULL, BAD_CAST("X509Certificate"), NULL);
			xmlTextWriterWriteBase64( writer, (const char *)cert->content, 0, cert->len);
			xmlTextWriterEndElement(writer);						
		}
		
		/* </X509Data> */
		xmlTextWriterEndElement(writer);			
	}
	
	/* </KeyInfo> */
	xmlTextWriterEndElement(writer);	
	
	
	/* </signature> */
	xmlTextWriterEndElement(writer);	
	
	/* serialize the next signature */
	if( XAR_SIGNATURE(sig)->next )
		xar_signature_serialize(XAR_SIGNATURE(sig)->next,writer);
	
	return 0;
}

void _xar_signature_remove_cert(struct __xar_x509cert_t *cert)
{
	struct __xar_x509cert_t *next;
	
	if( !cert )
		return;
	
	next = cert->next;
	
	if( cert->content )
		free(cert->content);
	
	free(cert);
	
	_xar_signature_remove_cert(next);
	
	return;
}

void xar_signature_remove(xar_signature_t sig)
{
	xar_signature_t	next;
	
	if( !sig )
		return;
	
	next = XAR_SIGNATURE(sig)->next;

	if( XAR_SIGNATURE(sig)->type )
		free(XAR_SIGNATURE(sig)->type);

	if( XAR_SIGNATURE(sig)->x509cert_count ){
		_xar_signature_remove_cert(XAR_SIGNATURE(sig)->x509certs);		
	}
	
	free(XAR_SIGNATURE(sig));
	
	/* remove the next one in the list */
	xar_signature_remove(next);
	
	return;
}
