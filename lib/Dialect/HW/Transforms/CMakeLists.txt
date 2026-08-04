add_circt_dialect_library(CIRCTHWTransforms
  HWAggregateToComb.cpp
  HWConvertBitcasts.cpp
  HWSplitBitVectorPorts.cpp
  HWBypassInnerSymbols.cpp
  HWIMDeadCodeElim.cpp
  HWParameterizeConstantPorts.cpp
  HWPrintInstanceGraph.cpp
  HWSpecialize.cpp
  PrintHWModuleGraph.cpp
  FlattenIO.cpp
  VerifyInnerRefNamespace.cpp
  FlattenModules.cpp
  HWVectorization.cpp

  DEPENDS
  CIRCTHWTransformsIncGen

  LINK_LIBS PUBLIC
  CIRCTHW
  CIRCTSV
  CIRCTSeq
  CIRCTComb
  CIRCTSupport
  MLIRIR
  MLIRPass
  MLIRTransformUtils
)
