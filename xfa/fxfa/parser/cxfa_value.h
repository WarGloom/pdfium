// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_VALUE_H_
#define XFA_FXFA_PARSER_CXFA_VALUE_H_

#include "core/fxcrt/fx_string.h"
#include "xfa/fxfa/parser/cxfa_arcdata.h"
#include "xfa/fxfa/parser/cxfa_datadata.h"
#include "xfa/fxfa/parser/cxfa_exdatadata.h"
#include "xfa/fxfa/parser/cxfa_imagedata.h"
#include "xfa/fxfa/parser/cxfa_linedata.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_rectangledata.h"
#include "xfa/fxfa/parser/cxfa_text.h"

class CXFA_Value : public CXFA_Node {
 public:
  CXFA_Value(CXFA_Document* doc, XFA_PacketType packet);
  ~CXFA_Value() override;

  XFA_Element GetChildValueClassID() const;
  WideString GetChildValueContent() const;
  CXFA_ArcData GetArcData() const;
  CXFA_LineData GetLineData() const;
  CXFA_RectangleData GetRectangleData() const;
  CXFA_Text* GetText() const;
  CXFA_ExDataData GetExData() const;
  CXFA_ImageData GetImageData() const;
};

#endif  // XFA_FXFA_PARSER_CXFA_VALUE_H_
