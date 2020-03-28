#ifndef MACERRORS_H
#define MACERRORS_H
#include <CoreFoundation/CFBase.h>
#include <MacTypes.h>

#ifdef DARLING
inline OSStatus makeOSStatus(int errNo) { return 100000 + errNo; }
#endif

enum {
  paramErr = -50,
  noHardwareErr = -200,
  notEnoughHardwareErr = -201,
  userCanceledErr = -128,
  qErr = -1,
  vTypErr = -2,
  corErr = -3,
  unimpErr = -4,
  SlpTypeErr = -5,
  seNoDB = -8,
  controlErr = -17,
  statusErr = -18,
  readErr = -19,
  writErr = -20,
  badUnitErr = -21,
  unitEmptyErr = -22,
  openErr = -23,
  closErr = -24,
  dRemovErr = -25,
  dInstErr = -26
};

enum {
  abortErr = -27,
  iIOAbortErr = -27,
  notOpenErr = -28,
  unitTblFullErr = -29,
  dceExtErr = -30,
  slotNumErr = -360,
  gcrOnMFMErr = -400,
  dirFulErr = -33,
  dskFulErr = -34,
  nsvErr = -35,
  ioErr = -36,
  bdNamErr = -37,
  fnOpnErr = -38,
  eofErr = -39,
  posErr = -40,
  mFulErr = -41,
  tmfoErr = -42,
  fnfErr = -43,
  wPrErr = -44,
  fLckdErr = -45
};

enum {
  vLckdErr = -46,
  fBsyErr = -47,
  dupFNErr = -48,
  opWrErr = -49,
  rfNumErr = -51,
  gfpErr = -52,
  volOffLinErr = -53,
  permErr = -54,
  volOnLinErr = -55,
  nsDrvErr = -56,
  noMacDskErr = -57,
  extFSErr = -58,
  fsRnErr = -59,
  badMDBErr = -60,
  wrPermErr = -61,
  dirNFErr = -120,
  tmwdoErr = -121,
  badMovErr = -122,
  wrgVolTypErr = -123,
  volGoneErr = -124
};

enum {
  fidNotFound = -1300,
  fidExists = -1301,
  notAFileErr = -1302,
  diffVolErr = -1303,
  catChangedErr = -1304,
  desktopDamagedErr = -1305,
  sameFileErr = -1306,
  badFidErr = -1307,
  notARemountErr = -1308,
  fileBoundsErr = -1309,
  fsDataTooBigErr = -1310,
  volVMBusyErr = -1311,
  badFCBErr = -1327,
  errFSUnknownCall = -1400,
  errFSBadFSRef = -1401,
  errFSBadForkName = -1402,
  errFSBadBuffer = -1403,
  errFSBadForkRef = -1404,
  errFSBadInfoBitmap = -1405,
  errFSMissingCatInfo = -1406,
  errFSNotAFolder = -1407,
  errFSForkNotFound = -1409,
  errFSNameTooLong = -1410,
  errFSMissingName = -1411,
  errFSBadPosMode = -1412,
  errFSBadAllocFlags = -1413,
  errFSNoMoreItems = -1417,
  errFSBadItemCount = -1418,
  errFSBadSearchParams = -1419,
  errFSRefsDifferent = -1420,
  errFSForkExists = -1421,
  errFSBadIteratorFlags = -1422,
  errFSIteratorNotFound = -1423,
  errFSIteratorNotSupported = -1424,
  errFSQuotaExceeded = -1425,
  errFSOperationNotSupported = -1426,
  errFSAttributeNotFound = -1427,
  errFSPropertyNotValid = -1428,
  errFSNotEnoughSpaceForOperation = -1429,
  envNotPresent = -5500,
  envBadVers = -5501,
  envVersTooBig = -5502,
  fontDecError = -64,
  fontNotDeclared = -65,
  fontSubErr = -66,
  fontNotOutlineErr = -32615,
  firstDskErr = -84,
  lastDskErr = -64,
  noDriveErr = -64,
  offLinErr = -65,
  noNybErr = -66
};

enum {
  noAdrMkErr = -67,
  dataVerErr = -68,
  badCksmErr = -69,
  badBtSlpErr = -70,
  noDtaMkErr = -71,
  badDCksum = -72,
  badDBtSlp = -73,
  wrUnderrun = -74,
  cantStepErr = -75,
  tk0BadErr = -76,
  initIWMErr = -77,
  twoSideErr = -78,
  spdAdjErr = -79,
  seekErr = -80,
  sectNFErr = -81,
  fmt1Err = -82,
  fmt2Err = -83,
  verErr = -84,
  clkRdErr = -85,
  clkWrErr = -86,
  prWrErr = -87,
  prInitErr = -88,
  rcvrErr = -89,
  breakRecd = -90
};

enum {

  noScrapErr = -100,
  noTypeErr = -102
};

enum {

  eLenErr = -92,
  eMultiErr = -91
};

enum {
  ddpSktErr = -91,
  ddpLenErr = -92,
  noBridgeErr = -93,
  lapProtErr = -94,
  excessCollsns = -95,
  portNotPwr = -96,
  portInUse = -97,
  portNotCf = -98
};

enum {

  memROZWarn = -99,
  memROZError = -99,
  memROZErr = -99,
  memFullErr = -108,
  nilHandleErr = -109,
  memWZErr = -111,
  memPurErr = -112,
  memAdrErr = -110,
  memAZErr = -113,
  memPCErr = -114,
  memBCErr = -115,
  memSCErr = -116,
  memLockedErr = -117
};

enum {

  iMemFullErr = -108,
  iIOAbort = -27
};


enum {
  resourceInMemory = -188,
  writingPastEnd = -189,
  inputOutOfBounds = -190,
  resNotFound = -192,
  resFNotFound = -193,
  addResFailed = -194,
  addRefFailed = -195,
  rmvResFailed = -196,
  rmvRefFailed = -197,
  resAttrErr = -198,
  mapReadErr = -199,
  CantDecompress = -186,
  badExtResource = -185,
  noMemForPictPlaybackErr = -145,
  rgnOverflowErr = -147,
  rgnTooBigError = -147,
  pixMapTooDeepErr = -148,
  insufficientStackErr = -149,
  nsStackErr = -149
};

enum {
  evtNotEnb = 1
};


enum {
  cMatchErr = -150,
  cTempMemErr = -151,
  cNoMemErr = -152,
  cRangeErr = -153,
  cProtectErr = -154,
  cDevErr = -155,
  cResErr = -156,
  cDepthErr = -157,
  rgnTooBigErr = -500,
  updPixMemErr = -125,
  pictInfoVersionErr = -11000,
  pictInfoIDErr = -11001,
  pictInfoVerbErr = -11002,
  cantLoadPickMethodErr = -11003,
  colorsRequestedErr = -11004,
  pictureDataErr = -11005
};


enum {

  cmProfileError = -170,
  cmMethodError = -171,
  cmMethodNotFound = -175,
  cmProfileNotFound = -176,
  cmProfilesIdentical = -177,
  cmCantConcatenateError = -178,
  cmCantXYZ = -179,
  cmCantDeleteProfile = -180,
  cmUnsupportedDataType = -181,
  cmNoCurrentProfile = -182
};


enum {

  noHardware = noHardwareErr,
  notEnoughHardware = notEnoughHardwareErr,
  queueFull = -203,
  resProblem = -204,
  badChannel = -205,
  badFormat = -206,
  notEnoughBufferSpace = -207,
  badFileFormat = -208,
  channelBusy = -209,
  buffersTooSmall = -210,
  channelNotBusy = -211,
  noMoreRealTime = -212,
  siVBRCompressionNotSupported = -213,
  siNoSoundInHardware = -220,
  siBadSoundInDevice = -221,
  siNoBufferSpecified = -222,
  siInvalidCompression = -223,
  siHardDriveTooSlow = -224,
  siInvalidSampleRate = -225,
  siInvalidSampleSize = -226,
  siDeviceBusyErr = -227,
  siBadDeviceName = -228,
  siBadRefNum = -229,
  siInputDeviceErr = -230,
  siUnknownInfoType = -231,
  siUnknownQuality = -232
};


enum {
  noSynthFound = -240,
  synthOpenFailed = -241,
  synthNotReady = -242,
  bufTooSmall = -243,
  voiceNotFound = -244,
  incompatibleVoice = -245,
  badDictFormat = -246,
  badInputText = -247
};


enum {
  midiNoClientErr = -250,
  midiNoPortErr = -251,
  midiTooManyPortsErr = -252,
  midiTooManyConsErr = -253,
  midiVConnectErr = -254,
  midiVConnectMade = -255,
  midiVConnectRmvd = -256,
  midiNoConErr = -257,
  midiWriteErr = -258,
  midiNameLenErr = -259,
  midiDupIDErr = -260,
  midiInvalidCmdErr = -261
};


enum {
  nmTypErr = -299
};


enum {
  siInitSDTblErr = 1,
  siInitVBLQsErr = 2,
  siInitSPTblErr = 3,
  sdmJTInitErr = 10,
  sdmInitErr = 11,
  sdmSRTInitErr = 12,
  sdmPRAMInitErr = 13,
  sdmPriInitErr = 14
};

enum {
  smSDMInitErr = -290,
  smSRTInitErr = -291,
  smPRAMInitErr = -292,
  smPriInitErr = -293,
  smEmptySlot = -300,
  smCRCFail = -301,
  smFormatErr = -302,
  smRevisionErr = -303,
  smNoDir = -304,
  smDisabledSlot = -305,
  smNosInfoArray = -306
};


enum {
  smResrvErr = -307,
  smUnExBusErr = -308,
  smBLFieldBad = -309,
  smFHBlockRdErr = -310,
  smFHBlkDispErr = -311,
  smDisposePErr = -312,
  smNoBoardSRsrc = -313,
  smGetPRErr = -314,
  smNoBoardId = -315,
  smInitStatVErr = -316,
  smInitTblVErr = -317,
  smNoJmpTbl = -318,
  smReservedSlot = -318,
  smBadBoardId = -319,
  smBusErrTO = -320,

  svTempDisable = -32768L,
  svDisabled = -32640,
  smBadRefId = -330,
  smBadsList = -331,
  smReservedErr = -332,
  smCodeRevErr = -333
};

enum {
  smCPUErr = -334,
  smsPointerNil = -335,
  smNilsBlockErr = -336,
  smSlotOOBErr = -337,
  smSelOOBErr = -338,
  smNewPErr = -339,
  smBlkMoveErr = -340,
  smCkStatusErr = -341,
  smGetDrvrNamErr = -342,
  smDisDrvrNamErr = -343,
  smNoMoresRsrcs = -344,
  smsGetDrvrErr = -345,
  smBadsPtrErr = -346,
  smByteLanesErr = -347,
  smOffsetErr = -348,
  smNoGoodOpens = -349,
  smSRTOvrFlErr = -350,
  smRecNotFnd = -351
};


enum {

  notBTree = -410,
  btNoSpace = -413,
  btDupRecErr = -414,
  btRecNotFnd = -415,
  btKeyLenErr = -416,
  btKeyAttrErr = -417,
  unknownInsertModeErr = -20000,
  recordDataTooBigErr = -20001,
  invalidIndexErr = -20002
};





enum {
  fsmFFSNotFoundErr = -431,
  fsmBusyFFSErr = -432,
  fsmBadFFSNameErr = -433,
  fsmBadFSDLenErr = -434,
  fsmDuplicateFSIDErr = -435,
  fsmBadFSDVersionErr = -436,
  fsmNoAlternateStackErr = -437,
  fsmUnknownFSMMessageErr = -438
};


enum {

  editionMgrInitErr = -450,
  badSectionErr = -451,
  notRegisteredSectionErr = -452,
  badEditionFileErr = -453,
  badSubPartErr = -454,
  multiplePublisherWrn = -460,
  containerNotFoundWrn = -461,
  containerAlreadyOpenWrn = -462,
  notThePublisherWrn = -463
};

enum {
  teScrapSizeErr = -501,
  hwParamErr = -502,
  driverHardwareGoneErr = -503
};

enum {

  procNotFound = -600,
  memFragErr = -601,
  appModeErr = -602,
  protocolErr = -603,
  hardwareConfigErr = -604,
  appMemFullErr = -605,
  appIsDaemon = -606,
  bufferIsSmall = -607,
  noOutstandingHLE = -608,
  connectionInvalid = -609,
  noUserInteractionAllowed = -610
};

enum {

  wrongApplicationPlatform = -875,
  appVersionTooOld = -876,
  notAppropriateForClassic = -877
};


enum {
  threadTooManyReqsErr = -617,
  threadNotFoundErr = -618,
  threadProtocolErr = -619
};

enum {
  threadBadAppContextErr = -616
};


enum {
  notEnoughMemoryErr = -620,
  notHeldErr = -621,
  cannotMakeContiguousErr = -622,
  notLockedErr = -623,
  interruptsMaskedErr = -624,
  cannotDeferErr = -625,
  noMMUErr = -626
};


enum {
  vmMorePhysicalThanVirtualErr = -628,
  vmKernelMMUInitErr = -629,
  vmOffErr = -630,
  vmMemLckdErr = -631,
  vmBadDriver = -632,
  vmNoVectorErr = -633
};


enum {
  vmInvalidBackingFileIDErr = -640,
  vmMappingPrivilegesErr = -641,
  vmBusyBackingFileErr = -642,
  vmNoMoreBackingFilesErr = -643,
  vmInvalidFileViewIDErr = -644,
  vmFileViewAccessErr = -645,
  vmNoMoreFileViewsErr = -646,
  vmAddressNotInFileViewErr = -647,
  vmInvalidOwningProcessErr = -648
};


enum {
  rcDBNull = -800,
  rcDBValue = -801,
  rcDBError = -802,
  rcDBBadType = -803,
  rcDBBreak = -804,
  rcDBExec = -805,
  rcDBBadSessID = -806,
  rcDBBadSessNum = -807,
  rcDBBadDDEV = -808,
  rcDBAsyncNotSupp = -809,
  rcDBBadAsyncPB = -810,
  rcDBNoHandler = -811,
  rcDBWrongVersion = -812,
  rcDBPackNotInited = -813
};



enum {
  hmHelpDisabled = -850,
  hmBalloonAborted = -853,
  hmSameAsLastBalloon = -854,
  hmHelpManagerNotInited = -855,
  hmSkippedBalloon = -857,
  hmWrongVersion = -858,
  hmUnknownHelpType = -859,
  hmOperationUnsupported = -861,
  hmNoBalloonUp = -862,
  hmCloseViewActive = -863
};



enum {

  notInitErr = -900,
  nameTypeErr = -902,
  noPortErr = -903,


  noGlobalsErr = -904,
  localOnlyErr = -905,
  destPortErr = -906,
  sessTableErr = -907,
  noSessionErr = -908,
  badReqErr = -909,
  portNameExistsErr = -910,
  noUserNameErr = -911,
  userRejectErr = -912,
  noMachineNameErr = -913,
  noToolboxNameErr = -914,
  noResponseErr = -915,
  portClosedErr = -916,
  sessClosedErr = -917,
  badPortNameErr = -919,
  noDefaultUserErr = -922,
  notLoggedInErr = -923,
  noUserRefErr = -924,
  networkErr = -925,
  noInformErr = -926,
  authFailErr = -927,
  noUserRecErr = -928,
  badServiceMethodErr = -930,
  badLocNameErr = -931,
  guestNotAllowedErr = -932
};


enum {
  kFMIterationCompleted = -980L,
  kFMInvalidFontFamilyErr = -981L,
  kFMInvalidFontErr = -982L,
  kFMIterationScopeModifiedErr = -983L,
  kFMFontTableAccessErr = -984L,
  kFMFontContainerAccessErr = -985L
};

enum {
  noMaskFoundErr = -1000
};

enum {
  nbpBuffOvr = -1024,
  nbpNoConfirm = -1025,
  nbpConfDiff = -1026,
  nbpDuplicate = -1027,
  nbpNotFound = -1028,
  nbpNISErr = -1029
};

enum {
  aspBadVersNum = -1066,
  aspBufTooSmall = -1067,
  aspNoMoreSess = -1068,
  aspNoServers = -1069,
  aspParamErr = -1070,
  aspServerBusy = -1071,
  aspSessClosed = -1072,
  aspSizeErr = -1073,
  aspTooMany = -1074,
  aspNoAck = -1075
};

enum {
  reqFailed = -1096,
  tooManyReqs = -1097,
  tooManySkts = -1098,
  badATPSkt = -1099,
  badBuffNum = -1100,
  noRelErr = -1101,
  cbNotFound = -1102,
  noSendResp = -1103,
  noDataArea = -1104,
  reqAborted = -1105
};


enum {

  errRefNum = -1280,
  errAborted = -1279,
  errState = -1278,
  errOpening = -1277,
  errAttention = -1276,
  errFwdReset = -1275,
  errDSPQueueSize = -1274,
  errOpenDenied = -1273
};






enum {
  errAECoercionFail = -1700,
  errAEDescNotFound = -1701,
  errAECorruptData = -1702,
  errAEWrongDataType = -1703,
  errAENotAEDesc = -1704,
  errAEBadListItem = -1705,
  errAENewerVersion = -1706,
  errAENotAppleEvent = -1707,
  errAEEventNotHandled = -1708,
  errAEReplyNotValid = -1709,
  errAEUnknownSendMode = -1710,
  errAEWaitCanceled = -1711,
  errAETimeout = -1712,
  errAENoUserInteraction = -1713,
  errAENotASpecialFunction = -1714,
  errAEParamMissed = -1715,
  errAEUnknownAddressType = -1716,
  errAEHandlerNotFound = -1717,
  errAEReplyNotArrived = -1718,
  errAEIllegalIndex = -1719,
  errAEImpossibleRange = -1720,
  errAEWrongNumberArgs = -1721,
  errAEAccessorNotFound = -1723,
  errAENoSuchLogical = -1725,
  errAEBadTestKey = -1726,
  errAENotAnObjSpec = -1727,
  errAENoSuchObject = -1728,
  errAENegativeCount = -1729,
  errAEEmptyListContainer = -1730,
  errAEUnknownObjectType = -1731,
  errAERecordingIsAlreadyOn = -1732,
  errAEReceiveTerminate = -1733,
  errAEReceiveEscapeCurrent = -1734,
  errAEEventFiltered = -1735,
  errAEDuplicateHandler = -1736,
  errAEStreamBadNesting = -1737,
  errAEStreamAlreadyConverted = -1738,
  errAEDescIsNull = -1739,
  errAEBuildSyntaxError = -1740,
  errAEBufferTooSmall = -1741
};

enum {
  errOSASystemError = -1750,
  errOSAInvalidID = -1751,
  errOSABadStorageType = -1752,
  errOSAScriptError = -1753,
  errOSABadSelector = -1754,
  errOSASourceNotAvailable = -1756,
  errOSANoSuchDialect = -1757,
  errOSADataFormatObsolete = -1758,
  errOSADataFormatTooNew = -1759,
  errOSACorruptData = errAECorruptData,
  errOSARecordingIsAlreadyOn = errAERecordingIsAlreadyOn,
  errOSAComponentMismatch = -1761,
  errOSACantOpenComponent = -1762,
  errOSACantStorePointers = -1763
};




enum {
  errOffsetInvalid = -1800,
  errOffsetIsOutsideOfView = -1801,
  errTopOfDocument = -1810,
  errTopOfBody = -1811,
  errEndOfDocument = -1812,
  errEndOfBody = -1813
};


enum {

  badDragRefErr = -1850,
  badDragItemErr = -1851,
  badDragFlavorErr = -1852,
  duplicateFlavorErr = -1853,
  cantGetFlavorErr = -1854,
  duplicateHandlerErr = -1855,
  handlerNotFoundErr = -1856,
  dragNotAcceptedErr = -1857,
  unsupportedForPlatformErr = -1858,
  noSuitableDisplaysErr = -1859,
  badImageRgnErr = -1860,
  badImageErr = -1861,
  nonDragOriginatorErr = -1862
};



enum {
  couldNotResolveDataRef = -2000,
  badImageDescription = -2001,
  badPublicMovieAtom = -2002,
  cantFindHandler = -2003,
  cantOpenHandler = -2004,
  badComponentType = -2005,
  noMediaHandler = -2006,
  noDataHandler = -2007,
  invalidMedia = -2008,
  invalidTrack = -2009,
  invalidMovie = -2010,
  invalidSampleTable = -2011,
  invalidDataRef = -2012,
  invalidHandler = -2013,
  invalidDuration = -2014,
  invalidTime = -2015,
  cantPutPublicMovieAtom = -2016,
  badEditList = -2017,
  mediaTypesDontMatch = -2018,
  progressProcAborted = -2019,
  movieToolboxUninitialized = -2020,
  noRecordOfApp = movieToolboxUninitialized,
  wfFileNotFound = -2021,
  cantCreateSingleForkFile = -2022,
  invalidEditState = -2023,
  nonMatchingEditState = -2024,
  staleEditState = -2025,
  userDataItemNotFound = -2026,
  maxSizeToGrowTooSmall = -2027,
  badTrackIndex = -2028,
  trackIDNotFound = -2029,
  trackNotInMovie = -2030,
  timeNotInTrack = -2031,
  timeNotInMedia = -2032,
  badEditIndex = -2033,
  internalQuickTimeError = -2034,
  cantEnableTrack = -2035,
  invalidRect = -2036,
  invalidSampleNum = -2037,
  invalidChunkNum = -2038,
  invalidSampleDescIndex = -2039,
  invalidChunkCache = -2040,
  invalidSampleDescription = -2041,
  dataNotOpenForRead = -2042,
  dataNotOpenForWrite = -2043,
  dataAlreadyOpenForWrite = -2044,
  dataAlreadyClosed = -2045,
  endOfDataReached = -2046,
  dataNoDataRef = -2047,
  noMovieFound = -2048,
  invalidDataRefContainer = -2049,
  badDataRefIndex = -2050,
  noDefaultDataRef = -2051,
  couldNotUseAnExistingSample = -2052,
  featureUnsupported = -2053,
  noVideoTrackInMovieErr = -2054,
  noSoundTrackInMovieErr = -2055,
  soundSupportNotAvailableErr = -2056,
  unsupportedAuxiliaryImportData = -2057,
  auxiliaryExportDataUnavailable = -2058,
  samplesAlreadyInMediaErr = -2059,
  noSourceTreeFoundErr = -2060,
  sourceNotFoundErr = -2061,
  movieTextNotFoundErr = -2062,
  missingRequiredParameterErr = -2063,
  invalidSpriteWorldPropertyErr = -2064,
  invalidSpritePropertyErr = -2065,
  gWorldsNotSameDepthAndSizeErr = -2066,
  invalidSpriteIndexErr = -2067,
  invalidImageIndexErr = -2068,
  invalidSpriteIDErr = -2069
};

enum {
  internalComponentErr = -2070,
  notImplementedMusicOSErr = -2071,
  cantSendToSynthesizerOSErr = -2072,
  cantReceiveFromSynthesizerOSErr = -2073,
  illegalVoiceAllocationOSErr = -2074,
  illegalPartOSErr = -2075,
  illegalChannelOSErr = -2076,
  illegalKnobOSErr = -2077,
  illegalKnobValueOSErr = -2078,
  illegalInstrumentOSErr = -2079,
  illegalControllerOSErr = -2080,
  midiManagerAbsentOSErr = -2081,
  synthesizerNotRespondingOSErr = -2082,
  synthesizerOSErr = -2083,
  illegalNoteChannelOSErr = -2084,
  noteChannelNotAllocatedOSErr = -2085,
  tunePlayerFullOSErr = -2086,
  tuneParseOSErr = -2087,
  noExportProcAvailableErr = -2089,
  videoOutputInUseErr = -2090
};

enum {
  componentDllLoadErr = -2091,
  componentDllEntryNotFoundErr = -2092,
  qtmlDllLoadErr = -2093,
  qtmlDllEntryNotFoundErr = -2094,
  qtmlUninitialized = -2095,
  unsupportedOSErr = -2096,
  unsupportedProcessorErr = -2097,
  componentNotThreadSafeErr = -2098
};

enum {
  cannotFindAtomErr = -2101,
  notLeafAtomErr = -2102,
  atomsNotOfSameTypeErr = -2103,
  atomIndexInvalidErr = -2104,
  duplicateAtomTypeAndIDErr = -2105,
  invalidAtomErr = -2106,
  invalidAtomContainerErr = -2107,
  invalidAtomTypeErr = -2108,
  cannotBeLeafAtomErr = -2109,
  pathTooLongErr = -2110,
  emptyPathErr = -2111,
  noPathMappingErr = -2112,
  pathNotVerifiedErr = -2113,
  unknownFormatErr = -2114,
  wackBadFileErr = -2115,
  wackForkNotFoundErr = -2116,
  wackBadMetaDataErr = -2117,
  qfcbNotFoundErr = -2118,
  qfcbNotCreatedErr = -2119,
  AAPNotCreatedErr = -2120,
  AAPNotFoundErr = -2121,
  ASDBadHeaderErr = -2122,
  ASDBadForkErr = -2123,
  ASDEntryNotFoundErr = -2124,
  fileOffsetTooBigErr = -2125,
  notAllowedToSaveMovieErr = -2126,
  qtNetworkAlreadyAllocatedErr = -2127,
  urlDataHHTTPProtocolErr = -2129,
  urlDataHHTTPNoNetDriverErr = -2130,
  urlDataHHTTPURLErr = -2131,
  urlDataHHTTPRedirectErr = -2132,
  urlDataHFTPProtocolErr = -2133,
  urlDataHFTPShutdownErr = -2134,
  urlDataHFTPBadUserErr = -2135,
  urlDataHFTPBadPasswordErr = -2136,
  urlDataHFTPServerErr = -2137,
  urlDataHFTPDataConnectionErr = -2138,
  urlDataHFTPNoDirectoryErr = -2139,
  urlDataHFTPQuotaErr = -2140,
  urlDataHFTPPermissionsErr = -2141,
  urlDataHFTPFilenameErr = -2142,
  urlDataHFTPNoNetDriverErr = -2143,
  urlDataHFTPBadNameListErr = -2144,
  urlDataHFTPNeedPasswordErr = -2145,
  urlDataHFTPNoPasswordErr = -2146,
  urlDataHFTPServerDisconnectedErr = -2147,
  urlDataHFTPURLErr = -2148,
  notEnoughDataErr = -2149,
  qtActionNotHandledErr = -2157,
  qtXMLParseErr = -2158,
  qtXMLApplicationErr = -2159
};


enum {
  digiUnimpErr = -2201,
  qtParamErr = -2202,
  matrixErr = -2203,
  notExactMatrixErr = -2204,
  noMoreKeyColorsErr = -2205,
  notExactSizeErr = -2206,
  badDepthErr = -2207,
  noDMAErr = -2208,
  badCallOrderErr = -2209
};



enum {
  kernelIncompleteErr = -2401,
  kernelCanceledErr = -2402,
  kernelOptionsErr = -2403,
  kernelPrivilegeErr = -2404,
  kernelUnsupportedErr = -2405,
  kernelObjectExistsErr = -2406,
  kernelWritePermissionErr = -2407,
  kernelReadPermissionErr = -2408,
  kernelExecutePermissionErr = -2409,
  kernelDeletePermissionErr = -2410,
  kernelExecutionLevelErr = -2411,
  kernelAttributeErr = -2412,
  kernelAsyncSendLimitErr = -2413,
  kernelAsyncReceiveLimitErr = -2414,
  kernelTimeoutErr = -2415,
  kernelInUseErr = -2416,
  kernelTerminatedErr = -2417,
  kernelExceptionErr = -2418,
  kernelIDErr = -2419,
  kernelAlreadyFreeErr = -2421,
  kernelReturnValueErr = -2422,
  kernelUnrecoverableErr = -2499
};



enum {

  tsmComponentNoErr = 0,
  tsmUnsupScriptLanguageErr = -2500,
  tsmInputMethodNotFoundErr = -2501,
  tsmNotAnAppErr = -2502,
  tsmAlreadyRegisteredErr = -2503,
  tsmNeverRegisteredErr = -2504,
  tsmInvalidDocIDErr = -2505,
  tsmTSMDocBusyErr = -2506,
  tsmDocNotActiveErr = -2507,
  tsmNoOpenTSErr = -2508,
  tsmCantOpenComponentErr = -2509,
  tsmTextServiceNotFoundErr = -2510,
  tsmDocumentOpenErr = -2511,
  tsmUseInputWindowErr = -2512,
  tsmTSHasNoMenuErr = -2513,
  tsmTSNotOpenErr = -2514,
  tsmComponentAlreadyOpenErr = -2515,
  tsmInputMethodIsOldErr = -2516,
  tsmScriptHasNoIMErr = -2517,
  tsmUnsupportedTypeErr = -2518,
  tsmUnknownErr = -2519,
  tsmInvalidContext = -2520,
  tsmNoHandler = -2521,
  tsmNoMoreTokens = -2522,
  tsmNoStem = -2523,
  tsmDefaultIsNotInputMethodErr = -2524,
  tsmDocPropertyNotFoundErr = -2528,
  tsmDocPropertyBufferTooSmallErr = -2529,
  tsmCantChangeForcedClassStateErr = -2530,
  tsmComponentPropertyUnsupportedErr = -2531,
  tsmComponentPropertyNotFoundErr = -2532,
  tsmInputModeChangeFailedErr = -2533
};


enum {

  mmInternalError = -2526
};


enum {
  nrLockedErr = -2536,
  nrNotEnoughMemoryErr = -2537,
  nrInvalidNodeErr = -2538,
  nrNotFoundErr = -2539,
  nrNotCreatedErr = -2540,
  nrNameErr = -2541,
  nrNotSlotDeviceErr = -2542,
  nrDataTruncatedErr = -2543,
  nrPowerErr = -2544,
  nrPowerSwitchAbortErr = -2545,
  nrTypeMismatchErr = -2546,
  nrNotModifiedErr = -2547,
  nrOverrunErr = -2548,
  nrResultCodeBase = -2549,
  nrPathNotFound = -2550,
  nrPathBufferTooSmall = -2551,
  nrInvalidEntryIterationOp = -2552,
  nrPropertyAlreadyExists = -2553,
  nrIterationDone = -2554,
  nrExitedIteratorScope = -2555,
  nrTransactionAborted = -2556,
  nrCallNotSupported = -2557
};


enum {
  invalidIconRefErr = -2580,
  noSuchIconErr = -2581,
  noIconDataAvailableErr = -2582
};
enum {
  errOSACantCoerce = errAECoercionFail,
  errOSACantAccess = errAENoSuchObject,
  errOSACantAssign = -10006,
  errOSAGeneralError = -2700,
  errOSADivideByZero = -2701,
  errOSANumericOverflow = -2702,
  errOSACantLaunch = -2703,
  errOSAAppNotHighLevelEventAware = -2704,
  errOSACorruptTerminology = -2705,
  errOSAStackOverflow = -2706,
  errOSAInternalTableOverflow = -2707,
  errOSADataBlockTooLarge = -2708,
  errOSACantGetTerminology = -2709,
  errOSACantCreate = -2710
};
enum {
  errOSATypeError = errAEWrongDataType,
  OSAMessageNotUnderstood = errAEEventNotHandled,
  OSAUndefinedHandler = errAEHandlerNotFound,
  OSAIllegalAccess = errAEAccessorNotFound,
  OSAIllegalIndex = errAEIllegalIndex,
  OSAIllegalRange = errAEImpossibleRange,
  OSAIllegalAssign = -10003,
  OSASyntaxError = -2740,
  OSASyntaxTypeError = -2741,
  OSATokenTooLong = -2742,
  OSAMissingParameter = errAEDescNotFound,
  OSAParameterMismatch = errAEWrongNumberArgs,
  OSADuplicateParameter = -2750,
  OSADuplicateProperty = -2751,
  OSADuplicateHandler = -2752,
  OSAUndefinedVariable = -2753,
  OSAInconsistentDeclarations = -2754,
  OSAControlFlowError = -2755
};
enum {
  errASCantConsiderAndIgnore = -2720,
  errASCantCompareMoreThan32k = -2721,
  errASTerminologyNestingTooDeep = -2760,
  errASIllegalFormalParameter = -2761,
  errASParameterNotForEvent = -2762,
  errASNoResultReturned = -2763,
  errASInconsistentNames = -2780
};



enum {
  cfragFirstErrCode = -2800,
  cfragContextIDErr = -2800,
  cfragConnectionIDErr = -2801,
  cfragNoSymbolErr = -2802,
  cfragNoSectionErr = -2803,
  cfragNoLibraryErr = -2804,
  cfragDupRegistrationErr = -2805,
  cfragFragmentFormatErr = -2806,
  cfragUnresolvedErr = -2807,
  cfragNoPositionErr = -2808,
  cfragNoPrivateMemErr = -2809,
  cfragNoClientMemErr = -2810,
  cfragNoIDsErr = -2811,
  cfragInitOrderErr = -2812,
  cfragImportTooOldErr = -2813,
  cfragImportTooNewErr = -2814,
  cfragInitLoopErr = -2815,
  cfragInitAtBootErr = -2816,
  cfragLibConnErr = -2817,
  cfragCFMStartupErr = -2818,
  cfragCFMInternalErr = -2819,
  cfragFragmentCorruptErr = -2820,
  cfragInitFunctionErr = -2821,
  cfragNoApplicationErr = -2822,
  cfragArchitectureErr = -2823,
  cfragFragmentUsageErr = -2824,
  cfragFileSizeErr = -2825,
  cfragNotClosureErr = -2826,
  cfragNoRegistrationErr = -2827,
  cfragContainerIDErr = -2828,
  cfragClosureIDErr = -2829,
  cfragAbortClosureErr = -2830,
  cfragOutputLengthErr = -2831,
  cfragMapFileErr = -2851,
  cfragExecFileRefErr = -2854,
  cfragStdFolderErr = -2855,
  cfragRsrcForkErr = -2856,
  cfragCFragRsrcErr = -2857,
  cfragLastErrCode = -2899
};

enum {

  cfragFirstReservedCode = -2897,
  cfragReservedCode_3 = -2897,
  cfragReservedCode_2 = -2898,
  cfragReservedCode_1 = -2899
};
enum {
  invalidComponentID = -3000,
  validInstancesExist = -3001,
  componentNotCaptured = -3002,
  componentDontRegister = -3003,
  unresolvedComponentDLLErr = -3004,
  retryComponentRegistrationErr = -3005
};


enum {
  invalidTranslationPathErr = -3025,
  couldNotParseSourceFileErr = -3026,
  noTranslationPathErr = -3030,
  badTranslationSpecErr = -3031,
  noPrefAppErr = -3032
};

enum {
  buf2SmallErr = -3101,
  noMPPErr = -3102,
  ckSumErr = -3103,
  extractErr = -3104,
  readQErr = -3105,
  atpLenErr = -3106,
  atpBadRsp = -3107,
  recNotFnd = -3108,
  sktClosedErr = -3109
};



enum {
  kOTNoError = 0,
  kOTOutOfMemoryErr = -3211,
  kOTNotFoundErr = -3201,
  kOTDuplicateFoundErr = -3216,
  kOTBadAddressErr = -3150,
  kOTBadOptionErr = -3151,
  kOTAccessErr = -3152,
  kOTBadReferenceErr = -3153,
  kOTNoAddressErr = -3154,
  kOTOutStateErr = -3155,
  kOTBadSequenceErr = -3156,
  kOTSysErrorErr = -3157,
  kOTLookErr = -3158,
  kOTBadDataErr = -3159,
  kOTBufferOverflowErr = -3160,
  kOTFlowErr = -3161,
  kOTNoDataErr = -3162,
  kOTNoDisconnectErr = -3163,
  kOTNoUDErrErr = -3164,
  kOTBadFlagErr = -3165,
  kOTNoReleaseErr = -3166,
  kOTNotSupportedErr = -3167,
  kOTStateChangeErr = -3168,
  kOTNoStructureTypeErr = -3169,
  kOTBadNameErr = -3170,
  kOTBadQLenErr = -3171,
  kOTAddressBusyErr = -3172,
  kOTIndOutErr = -3173,
  kOTProviderMismatchErr = -3174,
  kOTResQLenErr = -3175,
  kOTResAddressErr = -3176,
  kOTQFullErr = -3177,
  kOTProtocolErr = -3178,
  kOTBadSyncErr = -3179,
  kOTCanceledErr = -3180,
  kEPERMErr = -3200,
  kENOENTErr = -3201,
  kENORSRCErr = -3202,
  kEINTRErr = -3203,
  kEIOErr = -3204,
  kENXIOErr = -3205,
  kEBADFErr = -3208,
  kEAGAINErr = -3210,
  kENOMEMErr = -3211,
  kEACCESErr = -3212,
  kEFAULTErr = -3213,
  kEBUSYErr = -3215,
  kEEXISTErr = -3216,
  kENODEVErr = -3218,
  kEINVALErr = -3221,
  kENOTTYErr = -3224,
  kEPIPEErr = -3231,
  kERANGEErr = -3233,
  kEWOULDBLOCKErr = -3234,
  kEDEADLKErr = -3234,
  kEALREADYErr = -3236,
  kENOTSOCKErr = -3237,
  kEDESTADDRREQErr = -3238,
  kEMSGSIZEErr = -3239,
  kEPROTOTYPEErr = -3240,
  kENOPROTOOPTErr = -3241,
  kEPROTONOSUPPORTErr = -3242,
  kESOCKTNOSUPPORTErr = -3243,
  kEOPNOTSUPPErr = -3244,
  kEADDRINUSEErr = -3247,
  kEADDRNOTAVAILErr = -3248,
  kENETDOWNErr = -3249,
  kENETUNREACHErr = -3250,
  kENETRESETErr = -3251,
  kECONNABORTEDErr = -3252,
  kECONNRESETErr = -3253,
  kENOBUFSErr = -3254,
  kEISCONNErr = -3255,
  kENOTCONNErr = -3256,
  kESHUTDOWNErr = -3257,
  kETOOMANYREFSErr = -3258,
  kETIMEDOUTErr = -3259,
  kECONNREFUSEDErr = -3260,
  kEHOSTDOWNErr = -3263,
  kEHOSTUNREACHErr = -3264,
  kEPROTOErr = -3269,
  kETIMEErr = -3270,
  kENOSRErr = -3271,
  kEBADMSGErr = -3272,
  kECANCELErr = -3273,
  kENOSTRErr = -3274,
  kENODATAErr = -3275,
  kEINPROGRESSErr = -3276,
  kESRCHErr = -3277,
  kENOMSGErr = -3278,
  kOTClientNotInittedErr = -3279,
  kOTPortHasDiedErr = -3280,
  kOTPortWasEjectedErr = -3281,
  kOTBadConfigurationErr = -3282,
  kOTConfigurationChangedErr = -3283,
  kOTUserRequestedErr = -3284,
  kOTPortLostConnection = -3285
};



enum {
  kQDNoPalette = -3950,
  kQDNoColorHWCursorSupport = -3951,
  kQDCursorAlreadyRegistered = -3952,
  kQDCursorNotRegistered = -3953,
  kQDCorruptPICTDataErr = -3954
};




enum {
  firstPickerError = -4000,
  invalidPickerType = firstPickerError,
  requiredFlagsDontMatch = -4001,
  pickerResourceError = -4002,
  cantLoadPicker = -4003,
  cantCreatePickerWindow = -4004,
  cantLoadPackage = -4005,
  pickerCantLive = -4006,
  colorSyncNotInstalled = -4007,
  badProfileError = -4008,
  noHelpForItem = -4009
};




enum {
  kNSL68kContextNotSupported = -4170,
  kNSLSchedulerError = -4171,
  kNSLBadURLSyntax = -4172,
  kNSLNoCarbonLib = -4173,
  kNSLUILibraryNotAvailable = -4174,
  kNSLNotImplementedYet = -4175,
  kNSLErrNullPtrError = -4176,
  kNSLSomePluginsFailedToLoad = -4177,
  kNSLNullNeighborhoodPtr = -4178,
  kNSLNoPluginsForSearch = -4179,
  kNSLSearchAlreadyInProgress = -4180,
  kNSLNoPluginsFound = -4181,
  kNSLPluginLoadFailed = -4182,
  kNSLBadProtocolTypeErr = -4183,
  kNSLNullListPtr = -4184,
  kNSLBadClientInfoPtr = -4185,
  kNSLCannotContinueLookup = -4186,
  kNSLBufferTooSmallForData = -4187,
  kNSLNoContextAvailable = -4188,
  kNSLRequestBufferAlreadyInList = -4189,
  kNSLInvalidPluginSpec = -4190,
  kNSLNoSupportForService = -4191,
  kNSLBadNetConnection = -4192,
  kNSLBadDataTypeErr = -4193,
  kNSLBadServiceTypeErr = -4194,
  kNSLBadReferenceErr = -4195,
  kNSLNoElementsInList = -4196,
  kNSLInsufficientOTVer = -4197,
  kNSLInsufficientSysVer = -4198,
  kNSLNotInitialized = -4199,
  kNSLInitializationFailed = -4200
};




enum {
  kDTPHoldJobErr = -4200,
  kDTPStopQueueErr = -4201,
  kDTPTryAgainErr = -4202,
  kDTPAbortJobErr = 128
};



enum {

  cmElementTagNotFound = -4200,
  cmIndexRangeErr = -4201,
  cmCantDeleteElement = -4202,
  cmFatalProfileErr = -4203,
  cmInvalidProfile = -4204,
  cmInvalidProfileLocation = -4205,
  cmCantCopyModifiedV1Profile = -4215,

  cmInvalidSearch = -4206,
  cmSearchError = -4207,
  cmErrIncompatibleProfile = -4208,
  cmInvalidColorSpace = -4209,
  cmInvalidSrcMap = -4210,
  cmInvalidDstMap = -4211,
  cmNoGDevicesError = -4212,
  cmInvalidProfileComment = -4213,
  cmRangeOverFlow = -4214,
  cmNamedColorNotFound = -4216,
  cmCantGamutCheckError = -4217
};


enum {
  badFolderDescErr = -4270,
  duplicateFolderDescErr = -4271,
  noMoreFolderDescErr = -4272,
  invalidFolderTypeErr = -4273,
  duplicateRoutingErr = -4274,
  routingNotFoundErr = -4275,
  badRoutingSizeErr = -4276
};



enum {
  coreFoundationUnknownErr = -4960
};


enum {
  errCoreEndianDataTooShortForFormat = -4940,
  errCoreEndianDataTooLongForFormat = -4941,
  errCoreEndianDataDoesNotMatchFormat = -4942
};



enum {
  internalScrapErr = -4988,
  duplicateScrapFlavorErr = -4989,
  badScrapRefErr = -4990,
  processStateIncorrectErr = -4991,
  scrapPromiseNotKeptErr = -4992,
  noScrapPromiseKeeperErr = -4993,
  nilScrapFlavorDataErr = -4994,
  scrapFlavorFlagsMismatchErr = -4995,
  scrapFlavorSizeMismatchErr = -4996,
  illegalScrapFlavorFlagsErr = -4997,
  illegalScrapFlavorTypeErr = -4998,
  illegalScrapFlavorSizeErr = -4999,
  scrapFlavorNotFoundErr = -102,
  needClearScrapErr = -100
};


enum {

  afpAccessDenied = -5000,
  afpAuthContinue = -5001,
  afpBadUAM = -5002,
  afpBadVersNum = -5003,
  afpBitmapErr = -5004,
  afpCantMove = -5005,
  afpDenyConflict = -5006,
  afpDirNotEmpty = -5007,
  afpDiskFull = -5008,
  afpEofError = -5009,
  afpFileBusy = -5010,
  afpFlatVol = -5011,
  afpItemNotFound = -5012,
  afpLockErr = -5013,
  afpMiscErr = -5014,
  afpNoMoreLocks = -5015,
  afpNoServer = -5016,
  afpObjectExists = -5017,
  afpObjectNotFound = -5018,
  afpParmErr = -5019,
  afpRangeNotLocked = -5020,
  afpRangeOverlap = -5021,
  afpSessClosed = -5022,
  afpUserNotAuth = -5023,
  afpCallNotSupported = -5024,
  afpObjectTypeErr = -5025,
  afpTooManyFilesOpen = -5026,
  afpServerGoingDown = -5027,
  afpCantRename = -5028,
  afpDirNotFound = -5029,
  afpIconTypeError = -5030,
  afpVolLocked = -5031,
  afpObjectLocked = -5032,
  afpContainsSharedErr = -5033,
  afpIDNotFound = -5034,
  afpIDExists = -5035,
  afpDiffVolErr = -5036,
  afpCatalogChanged = -5037,
  afpSameObjectErr = -5038,
  afpBadIDErr = -5039,
  afpPwdSameErr = -5040,
  afpPwdTooShortErr = -5041,
  afpPwdExpiredErr = -5042,
  afpInsideSharedErr = -5043,

  afpInsideTrashErr = -5044,

  afpPwdNeedsChangeErr = -5045,
  afpPwdPolicyErr = -5046,
  afpAlreadyLoggedInErr = -5047,
  afpCallNotAllowed = -5048
};

enum {

  afpBadDirIDType = -5060,
  afpCantMountMoreSrvre = -5061,
  afpAlreadyMounted = -5062,
  afpSameNodeErr = -5063
};






enum {
  numberFormattingNotANumberErr = -5200,
  numberFormattingOverflowInDestinationErr = -5201,
  numberFormattingBadNumberFormattingObjectErr = -5202,
  numberFormattingSpuriousCharErr = -5203,
  numberFormattingLiteralMissingErr = -5204,
  numberFormattingDelimiterMissingErr = -5205,
  numberFormattingEmptyFormatErr = -5206,
  numberFormattingBadFormatErr = -5207,
  numberFormattingBadOptionsErr = -5208,
  numberFormattingBadTokenErr = -5209,
  numberFormattingUnOrderedCurrencyRangeErr = -5210,
  numberFormattingBadCurrencyPositionErr = -5211,
  numberFormattingNotADigitErr = -5212,
  numberFormattingUnOrdredCurrencyRangeErr = -5210,
  numberFortmattingNotADigitErr = -5212
};


enum {
  textParserBadParamErr = -5220,
  textParserObjectNotFoundErr = -5221,
  textParserBadTokenValueErr = -5222,
  textParserBadParserObjectErr = -5223,
  textParserParamErr = -5224,
  textParserNoMoreTextErr = -5225,
  textParserBadTextLanguageErr = -5226,
  textParserBadTextEncodingErr = -5227,
  textParserNoSuchTokenFoundErr = -5228,
  textParserNoMoreTokensErr = -5229
};

enum {
  errUnknownAttributeTag = -5240,
  errMarginWilllNotFit = -5241,
  errNotInImagingMode = -5242,
  errAlreadyInImagingMode = -5243,
  errEngineNotFound = -5244,
  errIteratorReachedEnd = -5245,
  errInvalidRange = -5246,
  errOffsetNotOnElementBounday = -5247,
  errNoHiliteText = -5248,
  errEmptyScrap = -5249,
  errReadOnlyText = -5250,
  errUnknownElement = -5251,
  errNonContiuousAttribute = -5252,
  errCannotUndo = -5253
};



enum {
  hrHTMLRenderingLibNotInstalledErr = -5360,
  hrMiscellaneousExceptionErr = -5361,
  hrUnableToResizeHandleErr = -5362,
  hrURLNotHandledErr = -5363
};



enum {
  errIANoErr = 0,
  errIAUnknownErr = -5380,
  errIAAllocationErr = -5381,
  errIAParamErr = -5382,
  errIANoMoreItems = -5383,
  errIABufferTooSmall = -5384,
  errIACanceled = -5385,
  errIAInvalidDocument = -5386,
  errIATextExtractionErr = -5387,
  errIAEndOfTextRun = -5388
};



enum {
  qtsBadSelectorErr = -5400,
  qtsBadStateErr = -5401,
  qtsBadDataErr = -5402,
  qtsUnsupportedDataTypeErr = -5403,
  qtsUnsupportedRateErr = -5404,
  qtsUnsupportedFeatureErr = -5405,
  qtsTooMuchDataErr = -5406,
  qtsUnknownValueErr = -5407,
  qtsTimeoutErr = -5408,
  qtsConnectionFailedErr = -5420,
  qtsAddressBusyErr = -5421
};


enum {

  gestaltUnknownErr = -5550,
  gestaltUndefSelectorErr = -5551,
  gestaltDupSelectorErr = -5552,
  gestaltLocationErr = -5553
};



enum {
  menuPropertyInvalidErr = -5603,
  menuPropertyInvalid = menuPropertyInvalidErr,
  menuPropertyNotFoundErr = -5604,
  menuNotFoundErr = -5620,
  menuUsesSystemDefErr = -5621,
  menuItemNotFoundErr = -5622,
  menuInvalidErr = -5623
};



enum {
  errInvalidWindowPtr = -5600,
  errInvalidWindowRef = -5600,
  errUnsupportedWindowAttributesForClass = -5601,
  errWindowDoesNotHaveProxy = -5602,
  errInvalidWindowProperty = -5603,
  errWindowPropertyNotFound = -5604,
  errUnrecognizedWindowClass = -5605,
  errCorruptWindowDescription = -5606,
  errUserWantsToDragWindow = -5607,
  errWindowsAlreadyInitialized = -5608,
  errFloatingWindowsNotInitialized = -5609,
  errWindowNotFound = -5610,
  errWindowDoesNotFitOnscreen = -5611,
  windowAttributeImmutableErr = -5612,
  windowAttributesConflictErr = -5613,
  windowManagerInternalErr = -5614,
  windowWrongStateErr = -5615,
  windowGroupInvalidErr = -5616,
  windowAppModalStateAlreadyExistsErr = -5617,
  windowNoAppModalStateErr = -5618,
  errWindowDoesntSupportFocus = -30583,
  errWindowRegionCodeInvalid = -30593
};



enum {
  dialogNoTimeoutErr = -5640
};



enum {
  kNavWrongDialogStateErr = -5694,
  kNavWrongDialogClassErr = -5695,
  kNavInvalidSystemConfigErr = -5696,
  kNavCustomControlMessageFailedErr = -5697,
  kNavInvalidCustomControlMessageErr = -5698,
  kNavMissingKindStringErr = -5699
};



enum {
  collectionItemLockedErr = -5750,
  collectionItemNotFoundErr = -5751,
  collectionIndexRangeErr = -5752,
  collectionVersionErr = -5753
};



enum {
  kQTSSUnknownErr = -6150
};


enum {

  kDMGenErr = -6220,

  kDMMirroringOnAlready = -6221,
  kDMWrongNumberOfDisplays = -6222,
  kDMMirroringBlocked = -6223,
  kDMCantBlock = -6224,
  kDMMirroringNotOn = -6225,

  kSysSWTooOld = -6226,
  kDMSWNotInitializedErr = -6227,
  kDMDriverNotDisplayMgrAwareErr = -6228,
  kDMDisplayNotFoundErr = -6229,
  kDMNotFoundErr = -6229,
  kDMDisplayAlreadyInstalledErr = -6230,
  kDMMainDisplayCannotMoveErr = -6231,
  kDMNoDeviceTableclothErr = -6231,
  kDMFoundErr = -6232
};





enum {
  laTooSmallBufferErr = -6984,
  laEnvironmentBusyErr = -6985,
  laEnvironmentNotFoundErr = -6986,
  laEnvironmentExistErr = -6987,
  laInvalidPathErr = -6988,
  laNoMoreMorphemeErr = -6989,
  laFailAnalysisErr = -6990,
  laTextOverFlowErr = -6991,
  laDictionaryNotOpenedErr = -6992,
  laDictionaryUnknownErr = -6993,
  laDictionaryTooManyErr = -6994,
  laPropertyValueErr = -6995,
  laPropertyUnknownErr = -6996,
  laPropertyIsReadOnlyErr = -6997,
  laPropertyNotFoundErr = -6998,
  laPropertyErr = -6999,
  laEngineNotFoundErr = -7000
};


enum {
  kUSBNoErr = 0,
  kUSBNoTran = 0,
  kUSBNoDelay = 0,
  kUSBPending = 1
};
enum {
  kUSBNotSent2Err = -6901,
  kUSBNotSent1Err = -6902,
  kUSBBufUnderRunErr = -6903,
  kUSBBufOvrRunErr = -6904,
  kUSBRes2Err = -6905,
  kUSBRes1Err = -6906,
  kUSBUnderRunErr = -6907,
  kUSBOverRunErr = -6908,
  kUSBWrongPIDErr = -6909,
  kUSBPIDCheckErr = -6910,
  kUSBNotRespondingErr = -6911,
  kUSBEndpointStallErr = -6912,
  kUSBDataToggleErr = -6913,
  kUSBBitstufErr = -6914,
  kUSBCRCErr = -6915,
  kUSBLinkErr = -6916
};






enum {
  kUSBQueueFull = -6948,
  kUSBNotHandled = -6987,
  kUSBUnknownNotification = -6949,
  kUSBBadDispatchTable = -6950
};







enum {
  kUSBInternalReserved10 = -6951,
  kUSBInternalReserved9 = -6952,
  kUSBInternalReserved8 = -6953,
  kUSBInternalReserved7 = -6954,
  kUSBInternalReserved6 = -6955,
  kUSBInternalReserved5 = -6956,
  kUSBInternalReserved4 = -6957,
  kUSBInternalReserved3 = -6958,
  kUSBInternalReserved2 = -6959,
  kUSBInternalReserved1 = -6960
};


enum {
  kUSBPortDisabled = -6969,
  kUSBQueueAborted = -6970,
  kUSBTimedOut = -6971,
  kUSBDeviceDisconnected = -6972,
  kUSBDeviceNotSuspended = -6973,
  kUSBDeviceSuspended = -6974,
  kUSBInvalidBuffer = -6975,
  kUSBDevicePowerProblem = -6976,
  kUSBDeviceBusy = -6977,
  kUSBUnknownInterfaceErr = -6978,
  kUSBPipeStalledError = -6979,
  kUSBPipeIdleError = -6980,
  kUSBNoBandwidthError = -6981,
  kUSBAbortedError = -6982,
  kUSBFlagsError = -6983,
  kUSBCompletionError = -6984,
  kUSBPBLengthError = -6985,
  kUSBPBVersionError = -6986,
  kUSBNotFound = -6987,
  kUSBOutOfMemoryErr = -6988,
  kUSBDeviceErr = -6989,
  kUSBNoDeviceErr = -6990,
  kUSBAlreadyOpenErr = -6991,
  kUSBTooManyTransactionsErr = -6992,
  kUSBUnknownRequestErr = -6993,
  kUSBRqErr = -6994,
  kUSBIncorrectTypeErr = -6995,
  kUSBTooManyPipesErr = -6996,
  kUSBUnknownPipeErr = -6997,
  kUSBUnknownDeviceErr = -6998,
  kUSBInternalErr = -6999
};





enum {
  dcmParamErr = -7100,
  dcmNotDictionaryErr = -7101,
  dcmBadDictionaryErr = -7102,
  dcmPermissionErr = -7103,
  dcmDictionaryNotOpenErr = -7104,
  dcmDictionaryBusyErr = -7105,
  dcmBlockFullErr = -7107,
  dcmNoRecordErr = -7108,
  dcmDupRecordErr = -7109,
  dcmNecessaryFieldErr = -7110,
  dcmBadFieldInfoErr = -7111,
  dcmBadFieldTypeErr = -7112,
  dcmNoFieldErr = -7113,
  dcmBadKeyErr = -7115,
  dcmTooManyKeyErr = -7116,
  dcmBadDataSizeErr = -7117,
  dcmBadFindMethodErr = -7118,
  dcmBadPropertyErr = -7119,
  dcmProtectedErr = -7121,
  dcmNoAccessMethodErr = -7122,
  dcmBadFeatureErr = -7124,
  dcmIterationCompleteErr = -7126,
  dcmBufferOverflowErr = -7127
};



enum {
  kRAInvalidParameter = -7100,
  kRAInvalidPort = -7101,
  kRAStartupFailed = -7102,
  kRAPortSetupFailed = -7103,
  kRAOutOfMemory = -7104,
  kRANotSupported = -7105,
  kRAMissingResources = -7106,
  kRAIncompatiblePrefs = -7107,
  kRANotConnected = -7108,
  kRAConnectionCanceled = -7109,
  kRAUnknownUser = -7110,
  kRAInvalidPassword = -7111,
  kRAInternalError = -7112,
  kRAInstallationDamaged = -7113,
  kRAPortBusy = -7114,
  kRAUnknownPortState = -7115,
  kRAInvalidPortState = -7116,
  kRAInvalidSerialProtocol = -7117,
  kRAUserLoginDisabled = -7118,
  kRAUserPwdChangeRequired = -7119,
  kRAUserPwdEntryRequired = -7120,
  kRAUserInteractionRequired = -7121,
  kRAInitOpenTransportFailed = -7122,
  kRARemoteAccessNotReady = -7123,
  kRATCPIPInactive = -7124,
  kRATCPIPNotConfigured = -7125,
  kRANotPrimaryInterface = -7126,
  kRAConfigurationDBInitErr = -7127,
  kRAPPPProtocolRejected = -7128,
  kRAPPPAuthenticationFailed = -7129,
  kRAPPPNegotiationFailed = -7130,
  kRAPPPUserDisconnected = -7131,
  kRAPPPPeerDisconnected = -7132,
  kRAPeerNotResponding = -7133,
  kRAATalkInactive = -7134,
  kRAExtAuthenticationFailed = -7135,
  kRANCPRejectedbyPeer = -7136,
  kRADuplicateIPAddr = -7137,
  kRACallBackFailed = -7138,
  kRANotEnabled = -7139
};
enum {
  kATSUInvalidTextLayoutErr = -8790,


  kATSUInvalidStyleErr = -8791,


  kATSUInvalidTextRangeErr = -8792,


  kATSUFontsMatched = -8793,


  kATSUFontsNotMatched = -8794,


  kATSUNoCorrespondingFontErr = -8795,






  kATSUInvalidFontErr = -8796,
  kATSUInvalidAttributeValueErr = -8797,

  kATSUInvalidAttributeSizeErr = -8798,

  kATSUInvalidAttributeTagErr = -8799,

  kATSUInvalidCacheErr = -8800,




  kATSUNotSetErr = -8801,



  kATSUNoStyleRunsAssignedErr = -8802,

  kATSUQuickDrawTextErr = -8803,

  kATSULowLevelErr = -8804,

  kATSUNoFontCmapAvailableErr = -8805,

  kATSUNoFontScalerAvailableErr = -8806,

  kATSUCoordinateOverflowErr = -8807,

  kATSULineBreakInWord = -8808,


  kATSUBusyObjectErr = -8809
};
enum {

  kTextUnsupportedEncodingErr = -8738,
  kTextMalformedInputErr = -8739,
  kTextUndefinedElementErr = -8740,
  kTECMissingTableErr = -8745,
  kTECTableChecksumErr = -8746,
  kTECTableFormatErr = -8747,
  kTECCorruptConverterErr = -8748,
  kTECNoConversionPathErr = -8749,
  kTECBufferBelowMinimumSizeErr = -8750,
  kTECArrayFullErr = -8751,
  kTECBadTextRunErr = -8752,
  kTECPartialCharErr = -8753,
  kTECUnmappableElementErr = -8754,
  kTECIncompleteElementErr = -8755,
  kTECDirectionErr = -8756,
  kTECGlobalsUnavailableErr = -8770,
  kTECItemUnavailableErr = -8771,

  kTECUsedFallbacksStatus = -8783,
  kTECNeedFlushStatus = -8784,
  kTECOutputBufferFullStatus = -8785,

  unicodeChecksumErr = -8769,
  unicodeNoTableErr = -8768,
  unicodeVariantErr = -8767,
  unicodeFallbacksErr = -8766,
  unicodePartConvertErr = -8765,
  unicodeBufErr = -8764,
  unicodeCharErr = -8763,
  unicodeElementErr = -8762,
  unicodeNotFoundErr = -8761,
  unicodeTableFormatErr = -8760,
  unicodeDirectionErr = -8759,
  unicodeContextualErr = -8758,
  unicodeTextEncodingDataErr = -8757
};



enum {
  kUTCUnderflowErr = -8850,
  kUTCOverflowErr = -8851,
  kIllegalClockValueErr = -8852
};





enum {
  kATSUInvalidFontFallbacksErr = -8900,

  kATSUUnsupportedStreamFormatErr = -8901,

  kATSUBadStreamErr = -8902,


  kATSUOutputBufferTooSmallErr = -8903,

  kATSUInvalidCallInsideCallbackErr = -8904,

  kATSUNoFontNameErr = -8905,

  kATSULastErr = -8959
};



enum {
  codecErr = -8960,
  noCodecErr = -8961,
  codecUnimpErr = -8962,
  codecSizeErr = -8963,
  codecScreenBufErr = -8964,
  codecImageBufErr = -8965,
  codecSpoolErr = -8966,
  codecAbortErr = -8967,
  codecWouldOffscreenErr = -8968,
  codecBadDataErr = -8969,
  codecDataVersErr = -8970,
  codecExtensionNotFoundErr = -8971,
  scTypeNotFoundErr = codecExtensionNotFoundErr,
  codecConditionErr = -8972,
  codecOpenErr = -8973,
  codecCantWhenErr = -8974,
  codecCantQueueErr = -8975,
  codecNothingToBlitErr = -8976,
  codecNoMemoryPleaseWaitErr = -8977,
  codecDisabledErr = -8978,
  codecNeedToFlushChainErr = -8979,
  lockPortBitsBadSurfaceErr = -8980,
  lockPortBitsWindowMovedErr = -8981,
  lockPortBitsWindowResizedErr = -8982,
  lockPortBitsWindowClippedErr = -8983,
  lockPortBitsBadPortErr = -8984,
  lockPortBitsSurfaceLostErr = -8985,
  codecParameterDialogConfirm = -8986,
  codecNeedAccessKeyErr = -8987,
  codecOffscreenFailedErr = -8988,
  codecDroppedFrameErr = -8989,
  directXObjectAlreadyExists = -8990,
  lockPortBitsWrongGDeviceErr = -8991,
  codecOffscreenFailedPleaseRetryErr = -8992,
  badCodecCharacterizationErr = -8993,
  noThumbnailFoundErr = -8994
};



enum {
  kBadAdapterErr = -9050,
  kBadAttributeErr = -9051,
  kBadBaseErr = -9052,
  kBadEDCErr = -9053,
  kBadIRQErr = -9054,
  kBadOffsetErr = -9055,
  kBadPageErr = -9056,
  kBadSizeErr = -9057,
  kBadSocketErr = -9058,
  kBadTypeErr = -9059,
  kBadVccErr = -9060,
  kBadVppErr = -9061,
  kBadWindowErr = -9062,
  kBadArgLengthErr = -9063,
  kBadArgsErr = -9064,
  kBadHandleErr = -9065,
  kBadCISErr = -9066,
  kBadSpeedErr = -9067,
  kReadFailureErr = -9068,
  kWriteFailureErr = -9069,
  kGeneralFailureErr = -9070,
  kNoCardErr = -9071,
  kUnsupportedFunctionErr = -9072,
  kUnsupportedModeErr = -9073,
  kBusyErr = -9074,
  kWriteProtectedErr = -9075,
  kConfigurationLockedErr = -9076,
  kInUseErr = -9077,
  kNoMoreItemsErr = -9078,
  kOutOfResourceErr = -9079,
  kNoCardSevicesSocketsErr = -9080,
  kInvalidRegEntryErr = -9081,
  kBadLinkErr = -9082,
  kBadDeviceErr = -9083,
  k16BitCardErr = -9084,
  kCardBusCardErr = -9085,
  kPassCallToChainErr = -9086,
  kCantConfigureCardErr = -9087,
  kPostCardEventErr = -9088,
  kInvalidDeviceNumber = -9089,
  kUnsupportedVsErr = -9090,
  kInvalidCSClientErr = -9091,
  kBadTupleDataErr = -9092,
  kBadCustomIFIDErr = -9093,
  kNoIOWindowRequestedErr = -9094,
  kNoMoreTimerClientsErr = -9095,
  kNoMoreInterruptSlotsErr = -9096,
  kNoClientTableErr = -9097,
  kUnsupportedCardErr = -9098,
  kNoCardEnablersFoundErr = -9099,
  kNoEnablerForCardErr = -9100,
  kNoCompatibleNameErr = -9101,
  kClientRequestDenied = -9102,

  kNotReadyErr = -9103,
  kTooManyIOWindowsErr = -9104,
  kAlreadySavedStateErr = -9105,
  kAttemptDupCardEntryErr = -9106,
  kCardPowerOffErr = -9107,
  kNotZVCapableErr = -9108,
  kNoCardBusCISErr = -9109
};


enum {
  noDeviceForChannel = -9400,
  grabTimeComplete = -9401,
  cantDoThatInCurrentMode = -9402,
  notEnoughMemoryToGrab = -9403,
  notEnoughDiskSpaceToGrab = -9404,
  couldntGetRequiredComponent = -9405,
  badSGChannel = -9406,
  seqGrabInfoNotAvailable = -9407,
  deviceCantMeetRequest = -9408,
  badControllerHeight = -9994,
  editingNotAllowed = -9995,
  controllerBoundsNotExact = -9996,
  cannotSetWidthOfAttachedController = -9997,
  controllerHasFixedHeight = -9998,
  cannotMoveAttachedController = -9999
};


enum {
  errAEBadKeyForm = -10002,
  errAECantHandleClass = -10010,
  errAECantSupplyType = -10009,
  errAECantUndo = -10015,
  errAEEventFailed = -10000,
  errAEIndexTooLarge = -10007,
  errAEInTransaction = -10011,
  errAELocalOnly = -10016,
  errAENoSuchTransaction = -10012,
  errAENotAnElement = -10008,
  errAENotASingleObject = -10014,
  errAENotModifiable = -10003,
  errAENoUserSelection = -10013,
  errAEPrivilegeError = -10004,
  errAEReadDenied = -10005,
  errAETypeError = -10001,
  errAEWriteDenied = -10006,
  errAENotAnEnumMember = -10023,
  errAECantPutThatThere = -10024,
  errAEPropertiesClash = -10025
};


enum {
  telGenericError = -1,
  telNoErr = 0,
  telNoTools = 8,
  telBadTermErr = -10001,
  telBadDNErr = -10002,
  telBadCAErr = -10003,
  telBadHandErr = -10004,
  telBadProcErr = -10005,
  telCAUnavail = -10006,
  telNoMemErr = -10007,
  telNoOpenErr = -10008,
  telBadHTypeErr = -10010,
  telHTypeNotSupp = -10011,
  telBadLevelErr = -10012,
  telBadVTypeErr = -10013,
  telVTypeNotSupp = -10014,
  telBadAPattErr = -10015,
  telAPattNotSupp = -10016,
  telBadIndex = -10017,
  telIndexNotSupp = -10018,
  telBadStateErr = -10019,
  telStateNotSupp = -10020,
  telBadIntExt = -10021,
  telIntExtNotSupp = -10022,
  telBadDNDType = -10023,
  telDNDTypeNotSupp = -10024,
  telFeatNotSub = -10030,
  telFeatNotAvail = -10031,
  telFeatActive = -10032,
  telFeatNotSupp = -10033,
  telConfLimitErr = -10040,
  telConfNoLimit = -10041,
  telConfErr = -10042,
  telConfRej = -10043,
  telTransferErr = -10044,
  telTransferRej = -10045,
  telCBErr = -10046,
  telConfLimitExceeded = -10047,
  telBadDNType = -10050,
  telBadPageID = -10051,
  telBadIntercomID = -10052,
  telBadFeatureID = -10053,
  telBadFwdType = -10054,
  telBadPickupGroupID = -10055,
  telBadParkID = -10056,
  telBadSelect = -10057,
  telBadBearerType = -10058,
  telBadRate = -10059,
  telDNTypeNotSupp = -10060,
  telFwdTypeNotSupp = -10061,
  telBadDisplayMode = -10062,
  telDisplayModeNotSupp = -10063,
  telNoCallbackRef = -10064,
  telAlreadyOpen = -10070,
  telStillNeeded = -10071,
  telTermNotOpen = -10072,
  telCANotAcceptable = -10080,
  telCANotRejectable = -10081,
  telCANotDeflectable = -10082,
  telPBErr = -10090,
  telBadFunction = -10091,

  telNoSuchTool = -10102,
  telUnknownErr = -10103,
  telNoCommFolder = -10106,
  telInitFailed = -10107,
  telBadCodeResource = -10108,
  telDeviceNotFound = -10109,
  telBadProcID = -10110,
  telValidateFailed = -10111,
  telAutoAnsNotOn = -10112,
  telDetAlreadyOn = -10113,
  telBadSWErr = -10114,
  telBadSampleRate = -10115,
  telNotEnoughdspBW = -10116
};

enum {
  errTaskNotFound = -10780
};





enum {

  pmBusyErr = -13000,
  pmReplyTOErr = -13001,
  pmSendStartErr = -13002,
  pmSendEndErr = -13003,
  pmRecvStartErr = -13004,
  pmRecvEndErr = -13005
};


enum {
  kPowerHandlerExistsForDeviceErr = -13006,
  kPowerHandlerNotFoundForDeviceErr = -13007,
  kPowerHandlerNotFoundForProcErr = -13008,
  kPowerMgtMessageNotHandled = -13009,
  kPowerMgtRequestDenied = -13010,
  kCantReportProcessorTemperatureErr = -13013,
  kProcessorTempRoutineRequiresMPLib2 = -13014,
  kNoSuchPowerSource = -13020,
  kBridgeSoftwareRunningCantSleep = -13038
};



enum {
  debuggingExecutionContextErr = -13880,
  debuggingDuplicateSignatureErr = -13881,
  debuggingDuplicateOptionErr = -13882,
  debuggingInvalidSignatureErr = -13883,
  debuggingInvalidOptionErr = -13884,
  debuggingInvalidNameErr = -13885,
  debuggingNoCallbackErr = -13886,
  debuggingNoMatchErr = -13887
};



enum {
  kHIDVersionIncompatibleErr = -13909,
  kHIDDeviceNotReady = -13910
};



enum {
  kHIDSuccess = 0,
  kHIDInvalidRangePageErr = -13923,
  kHIDReportIDZeroErr = -13924,
  kHIDReportCountZeroErr = -13925,
  kHIDReportSizeZeroErr = -13926,
  kHIDUnmatchedDesignatorRangeErr = -13927,
  kHIDUnmatchedStringRangeErr = -13928,
  kHIDInvertedUsageRangeErr = -13929,
  kHIDUnmatchedUsageRangeErr = -13930,
  kHIDInvertedPhysicalRangeErr = -13931,
  kHIDInvertedLogicalRangeErr = -13932,
  kHIDBadLogicalMaximumErr = -13933,
  kHIDBadLogicalMinimumErr = -13934,
  kHIDUsagePageZeroErr = -13935,
  kHIDEndOfDescriptorErr = -13936,
  kHIDNotEnoughMemoryErr = -13937,
  kHIDBadParameterErr = -13938,
  kHIDNullPointerErr = -13939,
  kHIDInvalidReportLengthErr = -13940,
  kHIDInvalidReportTypeErr = -13941,
  kHIDBadLogPhysValuesErr = -13942,
  kHIDIncompatibleReportErr = -13943,
  kHIDInvalidPreparsedDataErr = -13944,
  kHIDNotValueArrayErr = -13945,
  kHIDUsageNotFoundErr = -13946,
  kHIDValueOutOfRangeErr = -13947,
  kHIDBufferTooSmallErr = -13948,
  kHIDNullStateErr = -13949,
  kHIDBaseError = -13950
};



enum {
  kModemOutOfMemory = -14000,
  kModemPreferencesMissing = -14001,
  kModemScriptMissing = -14002
};




enum {
  kTXNEndIterationErr = -22000,
  kTXNCannotAddFrameErr = -22001,
  kTXNInvalidFrameIDErr = -22002,
  kTXNIllegalToCrossDataBoundariesErr = -22003,
  kTXNUserCanceledOperationErr = -22004,
  kTXNBadDefaultFileTypeWarning = -22005,
  kTXNCannotSetAutoIndentErr = -22006,
  kTXNRunIndexOutofBoundsErr = -22007,
  kTXNNoMatchErr = -22008,
  kTXNAttributeTagInvalidForRunErr = -22009,
  kTXNSomeOrAllTagsInvalidForRunErr = -22010,
  kTXNInvalidRunIndex = -22011,
  kTXNAlreadyInitializedErr = -22012,
  kTXNCannotTurnTSMOffWhenUsingUnicodeErr = -22013,
  kTXNCopyNotAllowedInEchoModeErr = -22014,
  kTXNDataTypeNotAllowedErr = -22015,
  kTXNATSUIIsNotInstalledErr = -22016,
  kTXNOutsideOfLineErr = -22017,
  kTXNOutsideOfFrameErr = -22018
};





enum {
  printerStatusOpCodeNotSupportedErr = -25280
};



enum {
  errKCNotAvailable = -25291,
  errKCReadOnly = -25292,
  errKCAuthFailed = -25293,
  errKCNoSuchKeychain = -25294,
  errKCInvalidKeychain = -25295,
  errKCDuplicateKeychain = -25296,
  errKCDuplicateCallback = -25297,
  errKCInvalidCallback = -25298,
  errKCDuplicateItem = -25299,
  errKCItemNotFound = -25300,
  errKCBufferTooSmall = -25301,
  errKCDataTooLarge = -25302,
  errKCNoSuchAttr = -25303,
  errKCInvalidItemRef = -25304,
  errKCInvalidSearchRef = -25305,
  errKCNoSuchClass = -25306,
  errKCNoDefaultKeychain = -25307,
  errKCInteractionNotAllowed = -25308,
  errKCReadOnlyAttr = -25309,
  errKCWrongKCVersion = -25310,
  errKCKeySizeNotAllowed = -25311,
  errKCNoStorageModule = -25312,
  errKCNoCertificateModule = -25313,
  errKCNoPolicyModule = -25314,
  errKCInteractionRequired = -25315,
  errKCDataNotAvailable = -25316,
  errKCDataNotModifiable = -25317,
  errKCCreateChainFailed = -25318
};



enum {
  kUCOutputBufferTooSmall = -25340,
  kUCTextBreakLocatorMissingType = -25341
};

enum {
  kUCTSNoKeysAddedToObjectErr = -25342,
  kUCTSSearchListErr = -25343
};

enum {
  kUCTokenizerIterationFinished = -25344,
  kUCTokenizerUnknownLang = -25345,
  kUCTokenNotFound = -25346
};


enum {
  kMPIterationEndErr = -29275,
  kMPPrivilegedErr = -29276,
  kMPProcessCreatedErr = -29288,
  kMPProcessTerminatedErr = -29289,
  kMPTaskCreatedErr = -29290,
  kMPTaskBlockedErr = -29291,
  kMPTaskStoppedErr = -29292,
  kMPBlueBlockingErr = -29293,
  kMPDeletedErr = -29295,
  kMPTimeoutErr = -29296,
  kMPTaskAbortedErr = -29297,
  kMPInsufficientResourcesErr = -29298,
  kMPInvalidIDErr = -29299
};

enum {
  kMPNanokernelNeedsMemoryErr = -29294
};


enum {
  kCollateAttributesNotFoundErr = -29500,
  kCollateInvalidOptions = -29501,
  kCollateMissingUnicodeTableErr = -29502,
  kCollateUnicodeConvertFailedErr = -29503,
  kCollatePatternNotFoundErr = -29504,
  kCollateInvalidChar = -29505,
  kCollateBufferTooSmall = -29506,
  kCollateInvalidCollationRef = -29507
};



enum {
  kFNSInvalidReferenceErr = -29580,
  kFNSBadReferenceVersionErr = -29581,
  kFNSInvalidProfileErr = -29582,
  kFNSBadProfileVersionErr = -29583,
  kFNSDuplicateReferenceErr = -29584,
  kFNSMismatchErr = -29585,
  kFNSInsufficientDataErr = -29586,
  kFNSBadFlattenedSizeErr = -29587,
  kFNSNameNotFoundErr = -29589
};




enum {
  kLocalesBufferTooSmallErr = -30001,
  kLocalesTableFormatErr = -30002,
  kLocalesDefaultDisplayStatus = -30029
};



enum {
  kALMInternalErr = -30049,
  kALMGroupNotFoundErr = -30048,
  kALMNoSuchModuleErr = -30047,
  kALMModuleCommunicationErr = -30046,
  kALMDuplicateModuleErr = -30045,
  kALMInstallationErr = -30044,
  kALMDeferSwitchErr = -30043,
  kALMRebootFlagsLevelErr = -30042
};

enum {
  kALMLocationNotFoundErr = kALMGroupNotFoundErr
};



enum {
  kSSpInternalErr = -30340,
  kSSpVersionErr = -30341,
  kSSpCantInstallErr = -30342,
  kSSpParallelUpVectorErr = -30343,
  kSSpScaleToZeroErr = -30344
};



enum {
  kNSpInitializationFailedErr = -30360,
  kNSpAlreadyInitializedErr = -30361,
  kNSpTopologyNotSupportedErr = -30362,
  kNSpPipeFullErr = -30364,
  kNSpHostFailedErr = -30365,
  kNSpProtocolNotAvailableErr = -30366,
  kNSpInvalidGameRefErr = -30367,
  kNSpInvalidParameterErr = -30369,
  kNSpOTNotPresentErr = -30370,
  kNSpOTVersionTooOldErr = -30371,
  kNSpMemAllocationErr = -30373,
  kNSpAlreadyAdvertisingErr = -30374,
  kNSpNotAdvertisingErr = -30376,
  kNSpInvalidAddressErr = -30377,
  kNSpFreeQExhaustedErr = -30378,
  kNSpRemovePlayerFailedErr = -30379,
  kNSpAddressInUseErr = -30380,
  kNSpFeatureNotImplementedErr = -30381,
  kNSpNameRequiredErr = -30382,
  kNSpInvalidPlayerIDErr = -30383,
  kNSpInvalidGroupIDErr = -30384,
  kNSpNoPlayersErr = -30385,
  kNSpNoGroupsErr = -30386,
  kNSpNoHostVolunteersErr = -30387,
  kNSpCreateGroupFailedErr = -30388,
  kNSpAddPlayerFailedErr = -30389,
  kNSpInvalidDefinitionErr = -30390,
  kNSpInvalidProtocolRefErr = -30391,
  kNSpInvalidProtocolListErr = -30392,
  kNSpTimeoutErr = -30393,
  kNSpGameTerminatedErr = -30394,
  kNSpConnectFailedErr = -30395,
  kNSpSendFailedErr = -30396,
  kNSpMessageTooBigErr = -30397,
  kNSpCantBlockErr = -30398,
  kNSpJoinFailedErr = -30399
};



enum {
  kISpInternalErr = -30420,
  kISpSystemListErr = -30421,
  kISpBufferToSmallErr = -30422,
  kISpElementInListErr = -30423,
  kISpElementNotInListErr = -30424,
  kISpSystemInactiveErr = -30425,
  kISpDeviceInactiveErr = -30426,
  kISpSystemActiveErr = -30427,
  kISpDeviceActiveErr = -30428,
  kISpListBusyErr = -30429
};


enum {
  kDSpNotInitializedErr = -30440L,
  kDSpSystemSWTooOldErr = -30441L,
  kDSpInvalidContextErr = -30442L,
  kDSpInvalidAttributesErr = -30443L,
  kDSpContextAlreadyReservedErr = -30444L,
  kDSpContextNotReservedErr = -30445L,
  kDSpContextNotFoundErr = -30446L,
  kDSpFrameRateNotReadyErr = -30447L,
  kDSpConfirmSwitchWarning = -30448L,
  kDSpInternalErr = -30449L,
  kDSpStereoContextErr = -30450L
};







enum {
  kFBCvTwinExceptionErr = -30500,
  kFBCnoIndexesFound = -30501,
  kFBCallocFailed = -30502,
  kFBCbadParam = -30503,
  kFBCfileNotIndexed = -30504,
  kFBCbadIndexFile = -30505,
  kFBCcompactionFailed = -30506,
  kFBCvalidationFailed = -30507,
  kFBCindexingFailed = -30508,
  kFBCcommitFailed = -30509,
  kFBCdeletionFailed = -30510,
  kFBCmoveFailed = -30511,
  kFBCtokenizationFailed = -30512,
  kFBCmergingFailed = -30513,
  kFBCindexCreationFailed = -30514,
  kFBCaccessorStoreFailed = -30515,
  kFBCaddDocFailed = -30516,
  kFBCflushFailed = -30517,
  kFBCindexNotFound = -30518,
  kFBCnoSearchSession = -30519,
  kFBCindexingCanceled = -30520,
  kFBCaccessCanceled = -30521,
  kFBCindexFileDestroyed = -30522,
  kFBCindexNotAvailable = -30523,
  kFBCsearchFailed = -30524,
  kFBCsomeFilesNotIndexed = -30525,
  kFBCillegalSessionChange = -30526,

  kFBCanalysisNotAvailable = -30527,
  kFBCbadIndexFileVersion = -30528,
  kFBCsummarizationCanceled = -30529,
  kFBCindexDiskIOFailed = -30530,
  kFBCbadSearchSession = -30531,
  kFBCnoSuchHit = -30532
};



enum {
  notAQTVRMovieErr = -30540,
  constraintReachedErr = -30541,
  callNotSupportedByNodeErr = -30542,
  selectorNotSupportedByNodeErr = -30543,
  invalidNodeIDErr = -30544,
  invalidViewStateErr = -30545,
  timeNotInViewErr = -30546,
  propertyNotSupportedByNodeErr = -30547,
  settingNotSupportedByNodeErr = -30548,
  limitReachedErr = -30549,
  invalidNodeFormatErr = -30550,
  invalidHotSpotIDErr = -30551,
  noMemoryNodeFailedInitialize = -30552,
  streamingNodeNotReadyErr = -30553,
  qtvrLibraryLoadErr = -30554,
  qtvrUninitialized = -30555
};



enum {
  themeInvalidBrushErr = -30560,
  themeProcessRegisteredErr = -30561,
  themeProcessNotRegisteredErr = -30562,
  themeBadTextColorErr = -30563,
  themeHasNoAccentsErr = -30564,
  themeBadCursorIndexErr = -30565,
  themeScriptFontNotFoundErr = -30566,
  themeMonitorDepthNotSupportedErr = -30567,
  themeNoAppropriateBrushErr = -30568
};







enum {






  errMessageNotSupported = -30580,







  errDataNotSupported = -30581,






  errControlDoesntSupportFocus = -30582,







  errUnknownControl = -30584,
  errCouldntSetFocus = -30585,
  errNoRootControl = -30586,





  errRootAlreadyExists = -30587,





  errInvalidPartCode = -30588,
  errControlsAlreadyExist = -30589,





  errControlIsNotEmbedder = -30590,






  errDataSizeMismatch = -30591,






  errControlHiddenOrDisabled = -30592,






  errCantEmbedIntoSelf = -30594,






  errCantEmbedRoot = -30595,





  errItemNotControl = -30596,







  controlInvalidDataVersionErr = -30597,





  controlPropertyInvalid = -5603,






  controlPropertyNotFoundErr = -5604,




  controlHandleInvalidErr = -30599
};





enum {
  kURLInvalidURLReferenceError = -30770,
  kURLProgressAlreadyDisplayedError = -30771,
  kURLDestinationExistsError = -30772,
  kURLInvalidURLError = -30773,
  kURLUnsupportedSchemeError = -30774,
  kURLServerBusyError = -30775,
  kURLAuthenticationError = -30776,
  kURLPropertyNotYetKnownError = -30777,
  kURLUnknownPropertyError = -30778,
  kURLPropertyBufferTooSmallError = -30779,
  kURLUnsettablePropertyError = -30780,
  kURLInvalidCallError = -30781,
  kURLFileEmptyError = -30783,
  kURLExtensionFailureError = -30785,
  kURLInvalidConfigurationError = -30786,
  kURLAccessNotAvailableError = -30787,
  kURL68kNotSupportedError = -30788
};
enum {
  errCppGeneral = -32000,
  errCppbad_alloc = -32001,
  errCppbad_cast = -32002,
  errCppbad_exception = -32003,
  errCppbad_typeid = -32004,
  errCpplogic_error = -32005,
  errCppdomain_error = -32006,
  errCppinvalid_argument = -32007,
  errCpplength_error = -32008,
  errCppout_of_range = -32009,
  errCppruntime_error = -32010,
  errCppoverflow_error = -32011,
  errCpprange_error = -32012,
  errCppunderflow_error = -32013,
  errCppios_base_failure = -32014,
  errCppLastSystemDefinedError = -32020,
  errCppLastUserDefinedError = -32049
};


enum {
  badComponentInstance = (int)0x80008001,
  badComponentSelector = (int)0x80008002
};



enum {
  dsBusError = 1,
  dsAddressErr = 2,
  dsIllInstErr = 3,
  dsZeroDivErr = 4,
  dsChkErr = 5,
  dsOvflowErr = 6,
  dsPrivErr = 7,
  dsTraceErr = 8,
  dsLineAErr = 9,
  dsLineFErr = 10,
  dsMiscErr = 11,
  dsCoreErr = 12,
  dsIrqErr = 13,
  dsIOCoreErr = 14,
  dsLoadErr = 15,
  dsFPErr = 16,
  dsNoPackErr = 17,
  dsNoPk1 = 18,
  dsNoPk2 = 19
};

enum {
  dsNoPk3 = 20,
  dsNoPk4 = 21,
  dsNoPk5 = 22,
  dsNoPk6 = 23,
  dsNoPk7 = 24,
  dsMemFullErr = 25,
  dsBadLaunch = 26,
  dsFSErr = 27,
  dsStknHeap = 28,
  negZcbFreeErr = 33,
  dsFinderErr = 41,
  dsBadSlotInt = 51,
  dsBadSANEOpcode = 81,
  dsBadPatchHeader = 83,
  menuPrgErr = 84,
  dsMBarNFnd = 85,
  dsHMenuFindErr = 86,
  dsWDEFNotFound = 87,
  dsCDEFNotFound = 88,
  dsMDEFNotFound = 89
};

enum {
  dsNoFPU = 90,
  dsNoPatch = 98,
  dsBadPatch = 99,
  dsParityErr = 101,
  dsOldSystem = 102,
  ds32BitMode = 103,
  dsNeedToWriteBootBlocks = 104,
  dsNotEnoughRAMToBoot = 105,
  dsBufPtrTooLow = 106,
  dsVMDeferredFuncTableFull = 112,
  dsVMBadBackingStore = 113,
  dsCantHoldSystemHeap = 114,
  dsSystemRequiresPowerPC = 116,
  dsGibblyMovedToDisabledFolder = 117,
  dsUnBootableSystem = 118,
  dsMustUseFCBAccessors = 119,
  dsMacOSROMVersionTooOld = 120,
  dsLostConnectionToNetworkDisk = 121,
  dsRAMDiskTooBig = 122,
  dsWriteToSupervisorStackGuardPage = 128,
  dsReinsert = 30,
  shutDownAlert = 42,
  dsShutDownOrRestart = 20000,
  dsSwitchOffOrRestart = 20001,
  dsForcedQuit = 20002,
  dsRemoveDisk = 20003,
  dsDirtyDisk = 20004,
  dsShutDownOrResume = 20109,
  dsSCSIWarn = 20010,
  dsMBSysError = 29200,
  dsMBFlpySysError = 29201,
  dsMBATASysError = 29202,
  dsMBATAPISysError = 29203,
  dsMBExternFlpySysError = 29204,
  dsPCCardATASysError = 29205
};






enum {
  dsNoExtsMacsBug = -1,
  dsNoExtsDisassembler = -2,
  dsMacsBugInstalled = -10,
  dsDisassemblerInstalled = -11,
  dsExtensionsDisabled = -13,
  dsGreeting = 40,
  dsSysErr = 32767,

  WDEFNFnd = dsWDEFNotFound
};

enum {
  CDEFNFnd = dsCDEFNotFound,
  dsNotThe1 = 31,
  dsBadStartupDisk = 42,
  dsSystemFileErr = 43,
  dsHD20Installed = -12,
  mBarNFnd = -126,
  fsDSIntErr = -127,
  hMenuFindErr = -127,
  userBreak = -490,
  strUserBreak = -491,
  exUserBreak = -492
};


enum {

  dsBadLibrary = 1010,
  dsMixedModeFailure = 1011
};
enum {
  kPOSIXErrorBase = 100000,
  kPOSIXErrorEPERM = 100001,
  kPOSIXErrorENOENT = 100002,
  kPOSIXErrorESRCH = 100003,
  kPOSIXErrorEINTR = 100004,
  kPOSIXErrorEIO = 100005,
  kPOSIXErrorENXIO = 100006,
  kPOSIXErrorE2BIG = 100007,
  kPOSIXErrorENOEXEC = 100008,
  kPOSIXErrorEBADF = 100009,
  kPOSIXErrorECHILD = 100010,
  kPOSIXErrorEDEADLK = 100011,
  kPOSIXErrorENOMEM = 100012,
  kPOSIXErrorEACCES = 100013,
  kPOSIXErrorEFAULT = 100014,
  kPOSIXErrorENOTBLK = 100015,
  kPOSIXErrorEBUSY = 100016,
  kPOSIXErrorEEXIST = 100017,
  kPOSIXErrorEXDEV = 100018,
  kPOSIXErrorENODEV = 100019,
  kPOSIXErrorENOTDIR = 100020,
  kPOSIXErrorEISDIR = 100021,
  kPOSIXErrorEINVAL = 100022,
  kPOSIXErrorENFILE = 100023,
  kPOSIXErrorEMFILE = 100024,
  kPOSIXErrorENOTTY = 100025,
  kPOSIXErrorETXTBSY = 100026,
  kPOSIXErrorEFBIG = 100027,
  kPOSIXErrorENOSPC = 100028,
  kPOSIXErrorESPIPE = 100029,
  kPOSIXErrorEROFS = 100030,
  kPOSIXErrorEMLINK = 100031,
  kPOSIXErrorEPIPE = 100032,
  kPOSIXErrorEDOM = 100033,
  kPOSIXErrorERANGE = 100034,
  kPOSIXErrorEAGAIN = 100035,
  kPOSIXErrorEINPROGRESS = 100036,
  kPOSIXErrorEALREADY = 100037,
  kPOSIXErrorENOTSOCK = 100038,
  kPOSIXErrorEDESTADDRREQ = 100039,
  kPOSIXErrorEMSGSIZE = 100040,
  kPOSIXErrorEPROTOTYPE = 100041,
  kPOSIXErrorENOPROTOOPT = 100042,
  kPOSIXErrorEPROTONOSUPPORT = 100043,
  kPOSIXErrorESOCKTNOSUPPORT = 100044,
  kPOSIXErrorENOTSUP = 100045,
  kPOSIXErrorEPFNOSUPPORT = 100046,
  kPOSIXErrorEAFNOSUPPORT = 100047,
  kPOSIXErrorEADDRINUSE = 100048,
  kPOSIXErrorEADDRNOTAVAIL = 100049,
  kPOSIXErrorENETDOWN = 100050,
  kPOSIXErrorENETUNREACH = 100051,
  kPOSIXErrorENETRESET = 100052,
  kPOSIXErrorECONNABORTED = 100053,
  kPOSIXErrorECONNRESET = 100054,
  kPOSIXErrorENOBUFS = 100055,
  kPOSIXErrorEISCONN = 100056,
  kPOSIXErrorENOTCONN = 100057,
  kPOSIXErrorESHUTDOWN = 100058,
  kPOSIXErrorETOOMANYREFS = 100059,
  kPOSIXErrorETIMEDOUT = 100060,
  kPOSIXErrorECONNREFUSED = 100061,
  kPOSIXErrorELOOP = 100062,
  kPOSIXErrorENAMETOOLONG = 100063,
  kPOSIXErrorEHOSTDOWN = 100064,
  kPOSIXErrorEHOSTUNREACH = 100065,
  kPOSIXErrorENOTEMPTY = 100066,
  kPOSIXErrorEPROCLIM = 100067,
  kPOSIXErrorEUSERS = 100068,
  kPOSIXErrorEDQUOT = 100069,
  kPOSIXErrorESTALE = 100070,
  kPOSIXErrorEREMOTE = 100071,
  kPOSIXErrorEBADRPC = 100072,
  kPOSIXErrorERPCMISMATCH = 100073,
  kPOSIXErrorEPROGUNAVAIL = 100074,
  kPOSIXErrorEPROGMISMATCH = 100075,
  kPOSIXErrorEPROCUNAVAIL = 100076,
  kPOSIXErrorENOLCK = 100077,
  kPOSIXErrorENOSYS = 100078,
  kPOSIXErrorEFTYPE = 100079,
  kPOSIXErrorEAUTH = 100080,
  kPOSIXErrorENEEDAUTH = 100081,
  kPOSIXErrorEPWROFF = 100082,
  kPOSIXErrorEDEVERR = 100083,
  kPOSIXErrorEOVERFLOW = 100084,
  kPOSIXErrorEBADEXEC = 100085,
  kPOSIXErrorEBADARCH = 100086,
  kPOSIXErrorESHLIBVERS = 100087,
  kPOSIXErrorEBADMACHO = 100088,
  kPOSIXErrorECANCELED = 100089,
  kPOSIXErrorEIDRM = 100090,
  kPOSIXErrorENOMSG = 100091,
  kPOSIXErrorEILSEQ = 100092,
  kPOSIXErrorENOATTR = 100093,
  kPOSIXErrorEBADMSG = 100094,
  kPOSIXErrorEMULTIHOP = 100095,
  kPOSIXErrorENODATA = 100096,
  kPOSIXErrorENOLINK = 100097,
  kPOSIXErrorENOSR = 100098,
  kPOSIXErrorENOSTR = 100099,
  kPOSIXErrorEPROTO = 100100,
  kPOSIXErrorETIME = 100101,
  kPOSIXErrorEOPNOTSUPP = 100102
};

#ifdef __cplusplus
extern "C" {
#endif

extern void SysError(short errorCode);

#ifdef __cplusplus
}
#endif

#endif
