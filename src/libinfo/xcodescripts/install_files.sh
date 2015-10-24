# exit immediately on failure
set -e -x

function InstallHeaders() {
	DESTDIR="$DSTROOT/$INSTALL_PATH_PREFIX/$1"
	shift
	install -d -o "$INSTALL_OWNER" -g "$INSTALL_GROUP" -m 0755 "$DESTDIR"
	install -o "$INSTALL_OWNER" -g "$INSTALL_GROUP" -m 0444 "$@" "$DESTDIR"
}

InstallHeaders /usr/include \
	gen.subproj/ifaddrs.h \
	lookup.subproj/aliasdb.h \
	lookup.subproj/bootparams.h \
	lookup.subproj/grp.h \
	lookup.subproj/netdb.h \
	lookup.subproj/printerdb.h \
	lookup.subproj/pwd.h \
	membership.subproj/membership.h \
	membership.subproj/ntsid.h

InstallHeaders /usr/local/include \
	gen.subproj/configuration_profile.h \
	lookup.subproj/ils.h \
	lookup.subproj/kvbuf.h \
	lookup.subproj/libinfo.h \
	lookup.subproj/si_data.h \
	lookup.subproj/si_module.h \
	lookup.subproj/thread_data.h

InstallHeaders /usr/local/include \
	lookup.subproj/netdb_async.h \
	membership.subproj/membershipPriv.h

InstallHeaders /usr/include/rpc \
	rpc.subproj/auth.h \
	rpc.subproj/auth_unix.h \
	rpc.subproj/clnt.h \
	rpc.subproj/pmap_clnt.h \
	rpc.subproj/pmap_prot.h \
	rpc.subproj/pmap_rmt.h \
	rpc.subproj/rpc.h \
	rpc.subproj/rpc_msg.h \
	rpc.subproj/svc.h \
	rpc.subproj/svc_auth.h \
	rpc.subproj/types.h \
	rpc.subproj/xdr.h

InstallHeaders /usr/include/rpcsvc \
	nis.subproj/yp_prot.h \
	nis.subproj/ypclnt.h

# Don't install man pages for installhdrs nor simulator builds
if [ "$ACTION" == "installhdrs" -o \
     "${RC_ProjectName%_Sim}" != "${RC_ProjectName}" ] ; then
	exit 0
fi

function InstallManPages() {
	for MANPAGE in "$@"; do
		SECTION=`basename "${MANPAGE/*./}"`
		MANDIR="$DSTROOT"/usr/share/man/man"$SECTION"
		install -d -o "$INSTALL_OWNER" -g "$INSTALL_GROUP" -m 0755 "$MANDIR"
		install -o "$INSTALL_OWNER" -g "$INSTALL_GROUP" -m 0444 "$MANPAGE" "$MANDIR"
	done
}

function LinkManPages() {
	MANPAGE=`basename "$1"`
	SECTION=`basename "${MANPAGE/*./}"`
	MANDIR="$DSTROOT"/usr/share/man/man"$SECTION"
	shift
	for LINK in "$@"; do
		ln -hf "$MANDIR/$MANPAGE" "$MANDIR/$LINK"
	done
}

InstallManPages \
	gen.subproj/getifaddrs.3 \
	gen.subproj/getifmaddrs.3 \
	gen.subproj/gethostbyname.3 \
	gen.subproj/getipnodebyname.3 \
	gen.subproj/getnetent.3 \
	gen.subproj/getprotoent.3 \
	gen.subproj/getservent.3 \
	gen.subproj/if_indextoname.3 \
	gen.subproj/inet6_rth_space.3 \
	gen.subproj/inet6_rthdr_space.3 \
	gen.subproj/inet6_option_space.3 \
	gen.subproj/inet6_opt_init.3

InstallManPages \
	lookup.subproj/bootparams.5 \
	lookup.subproj/gai_strerror.3 \
	lookup.subproj/getaddrinfo.3 \
	lookup.subproj/getfsent.3 \
	lookup.subproj/getgrent.3 \
	lookup.subproj/getgrouplist.3 \
	lookup.subproj/getnameinfo.3 \
	lookup.subproj/getnetgrent.3 \
	lookup.subproj/getpwent.3 \
	lookup.subproj/initgroups.3

InstallManPages \
	membership.subproj/mbr_check_membership.3 \
	membership.subproj/mbr_uid_to_uuid.3

InstallManPages \
	nis.subproj/yp_all.3 \
	nis.subproj/yp_bind.3 \
	nis.subproj/yp_first.3 \
	nis.subproj/yp_get_default_domain.3 \
	nis.subproj/yp_master.3 \
	nis.subproj/yp_match.3 \
	nis.subproj/yp_next.3 \
	nis.subproj/yp_order.3 \
	nis.subproj/yp_unbind.3 \
	nis.subproj/ypclnt.3 \
	nis.subproj/yperr_string.3 \
	nis.subproj/ypprot_err.3 \
	nis.subproj/yp.8

InstallManPages \
	rpc.subproj/bindresvport.3 \
	rpc.subproj/getrpcent.3 \
	rpc.subproj/getrpcport.3 \
	rpc.subproj/rpc.3 \
	rpc.subproj/xdr.3

InstallManPages \
	util.subproj/rcmd.3 \
	util.subproj/hosts.equiv.5

LinkManPages gethostbyname.3 \
	endhostent.3 gethostbyaddr.3 gethostbyname2.3 \
	gethostent.3 herror.3 hstrerror.3 sethostent.3

LinkManPages getifaddrs.3 \
	freeifaddrs.3

LinkManPages getipnodebyname.3 \
	freehostent.3 getipnodebyaddr.3

LinkManPages getnetent.3 \
	endnetent.3 getnetbyaddr.3 getnetbyname.3 setnetent.3

LinkManPages getprotoent.3 \
	endprotoent.3 getprotobyname.3 getprotobynumber.3 setprotoent.3

LinkManPages getservent.3 \
	endservent.3 getservbyname.3 getservbyport.3 setservent.3

LinkManPages if_indextoname.3 \
	if_freenameindex.3 if_nameindex.3 if_nametoindex.3

LinkManPages inet6_option_space.3 \
	inet6_option_alloc.3 inet6_option_append.3 \
	inet6_option_find.3 inet6_option_init.3 inet6_option_next.3

LinkManPages inet6_rthdr_space.3 \
	inet6_rthdr_add.3 inet6_rthdr_getaddr.3 \
	inet6_rthdr_getflags.3 inet6_rthdr_init.3 \
	inet6_rthdr_lasthop.3 inet6_rthdr_reverse.3 \
	inet6_rthdr_segments.3

LinkManPages inet6_opt_init.3 \
	inet6_opt_append.3 inet6_opt_finish.3 inet6_opt_set_val.3 \
	inet6_opt_next.3 inet6_opt_find.3 inet6_opt_get_val.3

LinkManPages getaddrinfo.3 \
	freeaddrinfo.3

LinkManPages getfsent.3 \
	endfsent.3 getfsfile.3 getfsspec.3 getfstype.3 setfsent.3

LinkManPages getgrent.3 \
	endgrent.3 getgrgid.3 getgrgid_r.3 getgrnam.3 \
	getgrnam_r.3 setgrent.3 setgroupent.3

LinkManPages getnetgrent.3 \
	endnetgrent.3 innetgr.3 setnetgrent.3

LinkManPages getpwent.3 \
	endpwent.3 getpwnam.3 getpwnam_r.3 getpwuid.3 \
	getpwuid_r.3 setpassent.3 setpwent.3 setpwfile.3 \
	getpwuuid.3 getpwuuid_r.3

LinkManPages mbr_uid_to_uuid.3 \
	mbr_gid_to_uuid.3 mbr_sid_to_uuid.3 mbr_uuid_to_id.3 mbr_uuid_to_sid.3 \
	mbr_sid_to_string.3 mbr_string_to_sid.3

LinkManPages mbr_check_membership.3 \
	mbr_check_service_membership.3

LinkManPages getrpcent.3 \
	getrpcbyname.3 getrpcbynumber.3 endrpcent.3 setrpcent.3

LinkManPages rpc.3 \
	auth_destroy.3 authnone_create.3 authunix_create.3 \
	authunix_create_default.3 callrpc.3 clnt_broadcast.3 \
	clnt_call.3 clnt_control.3 clnt_create.3 clnt_destroy.3 \
	clnt_freeres.3 clnt_geterr.3 clnt_pcreateerror.3 \
	clnt_perrno.3 clnt_perror.3 clnt_spcreateerror.3 \
	clnt_sperrno.3 clnt_sperror.3 clntraw_create.3 \
	clnttcp_create.3 clntudp_bufcreate.3 clntudp_create.3 \
	get_myaddress.3 pmap_getmaps.3 pmap_getport.3 pmap_rmtcall.3 \
	pmap_set.3 pmap_unset.3 regsterrpc.3 rpc_createerr.3 \
	svc_destroy.3 svc_fds.3 svc_fdset.3 svc_getargs.3 \
	svc_getcaller.3 svc_getreg.3 svc_getregset.3 svc_register.3 \
	svc_run.3 svc_sendreply.3 svc_unregister.3 svcerr_auth.3 \
	svcerr_decode.3 svcerr_noproc.3 svcerr_noprog.3 \
	svcerr_progvers.3 svcerr_systemerr.3 svcerr_weakauth.3 \
	svcfd_create.3 svcraw_create.3 svctcp_create.3 \
	svcudp_bufcreate.3 xdr_accepted_reply.3 xdr_authunix_parms.3 \
	xdr_callhdr.3 xdr_callmsg.3 xdr_opaque_auth.3 xdr_pmap.3 \
	xdr_pmaplist.3 xdr_rejected_reply.3 xdr_replymsg.3 \
	xprt_register.3 xprt_unregister.3

LinkManPages xdr.3 \
	xdr_array.3 xdr_bool.3 xdr_bytes.3 xdr_char.3 \
	xdr_destroy.3 xdr_double.3 xdr_enum.3 xdr_float.3 xdr_free.3 \
	xdr_getpos.3 xdr_inline.3 xdr_int.3 xdr_long.3 \
	xdrmem_create.3 xdr_opaque.3 xdr_pointer.3 xdrrec_create.3 \
	xdrrec_endofrecord.3 xdrrec_eof.3 xdrrec_skiprecord.3 \
	xdr_reference.3 xdr_setpos.3 xdr_short.3 xdrstdio_create.3 \
	xdr_string.3 xdr_u_char.3 xdr_u_long.3 xdr_u_short.3 \
	xdr_union.3 xdr_vector.3 xdr_void.3 xdr_wrapstring.3

LinkManPages rcmd.3 \
	iruserok.3 iruserok_sa.3 rcmd_af.3 rresvport.3 rresvport_af.3 ruserok.3

LinkManPages hosts.equiv.5 \
	.rhosts.5

