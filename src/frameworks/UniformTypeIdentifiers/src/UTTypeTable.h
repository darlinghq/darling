/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

// Built-in system type declarations, included (more than once) by UTType.m.
//
// UT_CORE_TYPE(constant, identifier, parents, extensions, MIME types, description)
// UT_BASE_TYPE(identifier, parents, extensions, MIME types, description)
//   A base type has no exported constant; it exists because a documented constant conforms to it.
//
// Lists are space separated; the first extension and MIME type are the preferred ones, and when
// several types share a tag, the earlier row wins a lookup. Identifiers and direct conformances
// follow Apple's public UTType constant documentation; a type whose documentation names no
// supertype has none here. Extensions and MIME types are the commonly registered ones.

// Base hierarchy
UT_CORE_TYPE(UTTypeItem, "public.item", "", "", "", "item")
UT_CORE_TYPE(UTTypeContent, "public.content", "", "", "", "content")
UT_CORE_TYPE(UTTypeCompositeContent, "public.composite-content", "public.content", "", "", "composite content")
UT_CORE_TYPE(UTTypeData, "public.data", "public.item", "", "", "data")
UT_CORE_TYPE(UTTypeDirectory, "public.directory", "public.item", "", "", "directory")
UT_CORE_TYPE(UTTypeResolvable, "com.apple.resolvable", "", "", "", "resolvable reference")
UT_CORE_TYPE(UTTypeSymbolicLink, "public.symlink", "public.item com.apple.resolvable", "", "", "symbolic link")
UT_CORE_TYPE(UTTypeAliasFile, "com.apple.alias-file", "public.data com.apple.resolvable", "", "", "alias")
UT_CORE_TYPE(UTTypeMountPoint, "public.mount-point", "public.item com.apple.resolvable", "", "", "mount point")
UT_CORE_TYPE(UTTypeURLBookmarkData, "com.apple.bookmark", "public.data com.apple.resolvable", "", "", "URL bookmark data")
UT_CORE_TYPE(UTTypeURL, "public.url", "public.data", "", "", "URL")
UT_CORE_TYPE(UTTypeFileURL, "public.file-url", "public.url", "", "", "file URL")
UT_CORE_TYPE(UTTypeExecutable, "public.executable", "public.item", "", "", "executable")
UT_CORE_TYPE(UTTypeDiskImage, "public.disk-image", "", "", "", "disk image")

// Text
UT_CORE_TYPE(UTTypeText, "public.text", "public.data public.content", "", "", "text")
UT_CORE_TYPE(UTTypePlainText, "public.plain-text", "public.text", "txt text", "text/plain", "plain text")
UT_CORE_TYPE(UTTypeUTF8PlainText, "public.utf8-plain-text", "public.plain-text", "", "", "UTF-8 plain text")
UT_CORE_TYPE(UTTypeUTF16ExternalPlainText, "public.utf16-external-plain-text", "public.plain-text", "", "", "UTF-16 plain text with external byte order")
UT_CORE_TYPE(UTTypeUTF16PlainText, "public.utf16-plain-text", "public.plain-text", "", "", "UTF-16 plain text")
UT_CORE_TYPE(UTTypeDelimitedText, "public.delimited-values-text", "public.text", "", "", "delimited text")
UT_CORE_TYPE(UTTypeCommaSeparatedText, "public.comma-separated-values-text", "public.delimited-values-text", "csv", "text/csv", "comma-separated values")
UT_CORE_TYPE(UTTypeTabSeparatedText, "public.tab-separated-values-text", "public.delimited-values-text", "tsv", "text/tab-separated-values", "tab-separated values")
UT_CORE_TYPE(UTTypeUTF8TabSeparatedText, "public.utf8-tab-separated-values-text", "public.delimited-values-text public.utf8-plain-text", "", "", "UTF-8 tab-separated values")
UT_CORE_TYPE(UTTypeRTF, "public.rtf", "public.text", "rtf", "text/rtf application/rtf", "rich text document")
UT_CORE_TYPE(UTTypeHTML, "public.html", "public.text", "html htm", "text/html", "HTML text")
UT_CORE_TYPE(UTTypeXML, "public.xml", "public.text", "xml", "application/xml text/xml", "XML text")
UT_CORE_TYPE(UTTypeYAML, "public.yaml", "public.text", "yaml yml", "application/yaml", "YAML text")
UT_CORE_TYPE(UTTypeJSON, "public.json", "public.text", "json", "application/json", "JSON text")
UT_CORE_TYPE(UTTypeLog, "public.log", "", "log", "", "log")
UT_CORE_TYPE(UTTypeVCard, "public.vcard", "public.text public.contact", "vcf vcard", "text/vcard", "vCard")

// Source code and scripts
UT_CORE_TYPE(UTTypeSourceCode, "public.source-code", "public.text", "", "", "source code")
UT_CORE_TYPE(UTTypeAssemblyLanguageSource, "public.assembly-source", "public.source-code", "s", "", "assembly source")
UT_CORE_TYPE(UTTypeCSource, "public.c-source", "public.source-code", "c", "", "C source")
UT_CORE_TYPE(UTTypeObjectiveCSource, "public.objective-c-source", "public.source-code", "m", "", "Objective-C source")
UT_CORE_TYPE(UTTypeSwiftSource, "public.swift-source", "public.source-code", "swift", "", "Swift source")
UT_CORE_TYPE(UTTypeCPlusPlusSource, "public.c-plus-plus-source", "public.source-code", "cpp cc cxx", "", "C++ source")
UT_CORE_TYPE(UTTypeObjectiveCPlusPlusSource, "public.objective-c-plus-plus-source", "public.source-code", "mm", "", "Objective-C++ source")
UT_CORE_TYPE(UTTypeCHeader, "public.c-header", "public.source-code", "h", "", "C header")
UT_CORE_TYPE(UTTypeCPlusPlusHeader, "public.c-plus-plus-header", "public.source-code", "hpp hh hxx", "", "C++ header")
UT_CORE_TYPE(UTTypeScript, "public.script", "public.source-code", "", "", "script")
UT_CORE_TYPE(UTTypeAppleScript, "com.apple.applescript.text", "public.script", "applescript", "", "AppleScript text")
UT_CORE_TYPE(UTTypeOSAScript, "com.apple.applescript.script", "public.script public.data", "scpt", "", "compiled script")
UT_CORE_TYPE(UTTypeOSAScriptBundle, "com.apple.applescript.script-bundle", "public.script com.apple.bundle com.apple.package", "scptd", "", "script bundle")
UT_CORE_TYPE(UTTypeJavaScript, "com.netscape.javascript-source", "public.source-code public.executable", "js", "text/javascript", "JavaScript source")
UT_CORE_TYPE(UTTypeShellScript, "public.shell-script", "public.script", "sh", "application/x-sh", "shell script")
UT_CORE_TYPE(UTTypePerlScript, "public.perl-script", "public.shell-script", "pl pm", "", "Perl script")
UT_CORE_TYPE(UTTypePythonScript, "public.python-script", "public.shell-script", "py", "text/x-python", "Python script")
UT_CORE_TYPE(UTTypeRubyScript, "public.ruby-script", "public.shell-script", "rb", "", "Ruby script")
UT_CORE_TYPE(UTTypePHPScript, "public.php-script", "public.shell-script", "php", "", "PHP script")
UT_CORE_TYPE(UTTypeMakefile, "public.make-source", "public.script", "", "", "Makefile")

// Documents and other content
UT_CORE_TYPE(UTTypePDF, "com.adobe.pdf", "public.data public.composite-content", "pdf", "application/pdf", "PDF document")
UT_CORE_TYPE(UTTypeRTFD, "com.apple.rtfd", "com.apple.package public.composite-content", "rtfd", "", "rich text document with attachments")
UT_CORE_TYPE(UTTypeFlatRTFD, "com.apple.flat-rtfd", "public.data public.composite-content", "", "", "flattened rich text document with attachments")
UT_CORE_TYPE(UTTypeWebArchive, "com.apple.webarchive", "public.data public.composite-content", "webarchive", "application/x-webarchive", "web archive")
UT_CORE_TYPE(UTTypeEPUB, "org.idpf.epub-container", "public.data public.composite-content", "epub", "application/epub+zip", "EPUB publication")
UT_CORE_TYPE(UTTypePresentation, "public.presentation", "public.composite-content", "", "", "presentation")
UT_CORE_TYPE(UTTypeSpreadsheet, "public.spreadsheet", "public.content", "", "", "spreadsheet")
UT_CORE_TYPE(UTTypeDatabase, "public.database", "", "", "", "database")
UT_CORE_TYPE(UTTypeMessage, "public.message", "", "", "", "message")
UT_CORE_TYPE(UTTypeEmailMessage, "public.email-message", "public.message", "eml", "message/rfc822", "email message")
UT_CORE_TYPE(UTTypeContact, "public.contact", "", "", "", "contact")
UT_CORE_TYPE(UTTypeCalendarEvent, "public.calendar-event", "", "", "", "calendar event")
UT_CORE_TYPE(UTTypeToDoItem, "public.to-do-item", "", "", "", "to-do item")
UT_CORE_TYPE(UTTypeBookmark, "public.bookmark", "", "", "", "bookmark")
UT_BASE_TYPE("public.stored-url", "", "", "", "stored URL")
UT_CORE_TYPE(UTTypeInternetLocation, "com.apple.internet-location", "public.data public.stored-url", "inetloc", "", "internet location")
UT_CORE_TYPE(UTTypeInternetShortcut, "com.microsoft.internet-shortcut", "public.data public.stored-url", "url", "", "internet shortcut")
UT_CORE_TYPE(UTTypeFont, "public.font", "public.data", "", "", "font")
UT_CORE_TYPE(UTTypePropertyList, "com.apple.property-list", "public.data", "plist", "", "property list")
UT_CORE_TYPE(UTTypeXMLPropertyList, "com.apple.xml-property-list", "public.xml com.apple.property-list", "", "", "XML property list")
UT_CORE_TYPE(UTTypeBinaryPropertyList, "com.apple.binary-property-list", "com.apple.property-list", "", "", "binary property list")
UT_CORE_TYPE(UTTypeX509Certificate, "public.x509-certificate", "public.data", "cer crt der", "application/pkix-cert", "X.509 certificate")
UT_CORE_TYPE(UTTypePKCS12, "com.rsa.pkcs-12", "public.data", "p12 pfx", "application/x-pkcs12", "PKCS #12 data")

// Archives
UT_CORE_TYPE(UTTypeArchive, "public.archive", "", "", "", "archive")
UT_CORE_TYPE(UTTypeZIP, "public.zip-archive", "public.data public.archive", "zip", "application/zip", "ZIP archive")
UT_CORE_TYPE(UTTypeGZIP, "org.gnu.gnu-zip-archive", "public.data public.archive", "gz gzip", "application/gzip application/x-gzip", "gzip archive")
UT_CORE_TYPE(UTTypeBZ2, "public.bzip2-archive", "public.data public.archive", "bz2", "application/x-bzip2", "bzip2 archive")
UT_CORE_TYPE(UTTypeAppleArchive, "com.apple.archive", "public.data public.archive", "aar", "", "Apple archive")

// Images
UT_CORE_TYPE(UTTypeImage, "public.image", "public.data public.content", "", "", "image")
UT_CORE_TYPE(UTTypePNG, "public.png", "public.image", "png", "image/png", "PNG image")
UT_CORE_TYPE(UTTypeJPEG, "public.jpeg", "public.image", "jpeg jpg", "image/jpeg", "JPEG image")
UT_CORE_TYPE(UTTypeGIF, "com.compuserve.gif", "public.image", "gif", "image/gif", "GIF image")
UT_CORE_TYPE(UTTypeTIFF, "public.tiff", "public.image", "tiff tif", "image/tiff", "TIFF image")
UT_CORE_TYPE(UTTypeBMP, "com.microsoft.bmp", "public.image", "bmp", "image/bmp", "BMP image")
UT_CORE_TYPE(UTTypeICO, "com.microsoft.ico", "public.image", "ico", "image/vnd.microsoft.icon", "Windows icon image")
UT_CORE_TYPE(UTTypeICNS, "com.apple.icns", "public.image", "icns", "", "Apple icon image")
UT_BASE_TYPE("public.heif-standard", "public.image", "", "", "HEIF-standard image")
UT_CORE_TYPE(UTTypeHEIF, "public.heif", "public.heif-standard", "heif", "image/heif", "HEIF image")
UT_CORE_TYPE(UTTypeHEIC, "public.heic", "public.heif-standard", "heic", "image/heic", "HEIC image")
UT_CORE_TYPE(UTTypeWebP, "org.webmproject.webp", "public.image", "webp", "image/webp", "WebP image")
UT_CORE_TYPE(UTTypeSVG, "public.svg-image", "public.image", "svg", "image/svg+xml", "SVG image")
UT_CORE_TYPE(UTTypeRAWImage, "public.camera-raw-image", "public.image", "", "", "camera raw image")
UT_CORE_TYPE(UTTypeLivePhoto, "com.apple.live-photo", "public.image", "", "", "Live Photo")

// Audio and video
UT_CORE_TYPE(UTTypeAudiovisualContent, "public.audiovisual-content", "public.data public.content", "", "", "audiovisual content")
UT_CORE_TYPE(UTTypeMovie, "public.movie", "public.audiovisual-content", "", "", "movie")
UT_CORE_TYPE(UTTypeVideo, "public.video", "public.movie", "", "", "video")
UT_CORE_TYPE(UTTypeAudio, "public.audio", "public.audiovisual-content", "", "", "audio")
UT_CORE_TYPE(UTTypeQuickTimeMovie, "com.apple.quicktime-movie", "public.movie", "mov qt", "video/quicktime", "QuickTime movie")
UT_CORE_TYPE(UTTypeMPEG, "public.mpeg", "public.movie", "mpeg mpg", "video/mpeg", "MPEG movie")
UT_CORE_TYPE(UTTypeMPEG2Video, "public.mpeg-2-video", "public.video", "m2v", "", "MPEG-2 video")
UT_CORE_TYPE(UTTypeMPEG2TransportStream, "public.mpeg-2-transport-stream", "public.movie", "ts m2ts", "video/mp2t", "MPEG-2 transport stream")
UT_CORE_TYPE(UTTypeMPEG4Movie, "public.mpeg-4", "public.movie", "mp4", "video/mp4", "MPEG-4 movie")
UT_BASE_TYPE("com.apple.m4v-video", "public.mpeg-4", "m4v", "video/x-m4v", "MPEG-4 video")
UT_CORE_TYPE(UTTypeAppleProtectedMPEG4Video, "com.apple.protected-mpeg-4-video", "com.apple.m4v-video", "", "", "protected MPEG-4 video")
UT_CORE_TYPE(UTTypeAVI, "public.avi", "public.movie", "avi", "video/x-msvideo", "AVI movie")
UT_CORE_TYPE(UTTypeMPEG4Audio, "public.mpeg-4-audio", "public.mpeg public.audio", "m4a", "audio/mp4", "MPEG-4 audio")
UT_CORE_TYPE(UTTypeAppleProtectedMPEG4Audio, "com.apple.protected-mpeg-4-audio", "public.audio", "m4p", "", "protected MPEG-4 audio")
UT_CORE_TYPE(UTTypeMP3, "public.mp3", "public.audio", "mp3", "audio/mpeg", "MP3 audio")
UT_BASE_TYPE("public.aifc-audio", "public.audio", "aifc", "", "AIFF-C audio")
UT_CORE_TYPE(UTTypeAIFF, "public.aiff-audio", "public.aifc-audio", "aiff aif", "audio/aiff", "AIFF audio")
UT_CORE_TYPE(UTTypeWAV, "com.microsoft.waveform-audio", "public.audio", "wav wave", "audio/wav audio/vnd.wave", "WAVE audio")
UT_CORE_TYPE(UTTypeMIDI, "public.midi-audio", "public.audio", "mid midi", "audio/midi", "MIDI audio")
UT_CORE_TYPE(UTTypePlaylist, "public.playlist", "", "", "", "playlist")
UT_CORE_TYPE(UTTypeM3UPlaylist, "public.m3u-playlist", "public.text public.playlist", "m3u m3u8", "audio/mpegurl", "M3U playlist")

// 3D and augmented reality content
UT_CORE_TYPE(UTType3DContent, "public.3d-content", "public.content", "", "", "3D content")
UT_CORE_TYPE(UTTypeUSD, "com.pixar.universal-scene-description", "public.3d-content public.data", "usd", "", "Universal Scene Description")
UT_CORE_TYPE(UTTypeUSDZ, "com.pixar.universal-scene-description-mobile", "public.3d-content public.data", "usdz", "model/vnd.usdz+zip", "Universal Scene Description package")
UT_CORE_TYPE(UTTypeRealityFile, "com.apple.reality", "public.data", "reality", "", "Reality Composer file")
UT_CORE_TYPE(UTTypeSceneKitScene, "com.apple.scenekit.scene", "public.3d-content public.data", "scn", "", "SceneKit scene")
UT_CORE_TYPE(UTTypeARReferenceObject, "com.apple.arobject", "public.data", "arobject", "", "AR reference object")

// Directories, bundles and executables
UT_CORE_TYPE(UTTypeFolder, "public.folder", "public.directory", "", "", "folder")
UT_CORE_TYPE(UTTypeVolume, "public.volume", "public.folder", "", "", "volume")
UT_CORE_TYPE(UTTypePackage, "com.apple.package", "public.directory", "", "", "package")
UT_CORE_TYPE(UTTypeBundle, "com.apple.bundle", "public.directory", "", "", "bundle")
UT_CORE_TYPE(UTTypePluginBundle, "com.apple.plugin", "com.apple.bundle com.apple.package", "plugin", "", "plug-in")
UT_CORE_TYPE(UTTypeSpotlightImporter, "com.apple.metadata-importer", "com.apple.plugin", "mdimporter", "", "Spotlight importer")
UT_CORE_TYPE(UTTypeQuickLookGenerator, "com.apple.quicklook-generator", "com.apple.plugin", "qlgenerator", "", "Quick Look generator")
UT_CORE_TYPE(UTTypeXPCService, "com.apple.xpc-service", "com.apple.bundle com.apple.package", "xpc", "", "XPC service")
UT_CORE_TYPE(UTTypeFramework, "com.apple.framework", "com.apple.bundle", "framework", "", "framework")
UT_CORE_TYPE(UTTypeApplication, "com.apple.application", "public.executable", "", "", "application")
UT_CORE_TYPE(UTTypeApplicationBundle, "com.apple.application-bundle", "com.apple.application com.apple.bundle com.apple.package", "app", "", "application bundle")
UT_CORE_TYPE(UTTypeApplicationExtension, "com.apple.application-and-system-extension", "com.apple.xpc-service", "appex", "", "app extension")
UT_CORE_TYPE(UTTypeSystemPreferencesPane, "com.apple.systempreference.prefpane", "com.apple.package com.apple.bundle", "prefPane", "", "settings pane")
UT_CORE_TYPE(UTTypeUnixExecutable, "public.unix-executable", "public.data public.executable", "", "", "Unix executable")
UT_CORE_TYPE(UTTypeEXE, "public.windows-executable", "public.data public.executable", "exe", "", "Windows executable")
