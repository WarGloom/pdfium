// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/font/cpdf_tounicodemap.h"

#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fxcrt/retain_ptr.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/span.h"

TEST(cpdf_tounicodemap, StringToCode) {
  EXPECT_THAT(CPDF_ToUnicodeMap::StringToCode("<0001>"), testing::Optional(1u));
  EXPECT_THAT(CPDF_ToUnicodeMap::StringToCode("<c2>"), testing::Optional(194u));
  EXPECT_THAT(CPDF_ToUnicodeMap::StringToCode("<A2>"), testing::Optional(162u));
  EXPECT_THAT(CPDF_ToUnicodeMap::StringToCode("<Af2>"),
              testing::Optional(2802u));
  EXPECT_THAT(CPDF_ToUnicodeMap::StringToCode("<FFFFFFFF>"),
              testing::Optional(4294967295u));

  // Integer overflow
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("<100000000>").has_value());
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("<1abcdFFFF>").has_value());

  // Invalid string
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("").has_value());
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("<>").has_value());
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("12").has_value());
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("<12").has_value());
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("12>").has_value());
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("<1-7>").has_value());
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("00AB").has_value());
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("<00NN>").has_value());
}

TEST(cpdf_tounicodemap, StringToWideString) {
  EXPECT_EQ(L"", CPDF_ToUnicodeMap::StringToWideString(""));
  EXPECT_EQ(L"", CPDF_ToUnicodeMap::StringToWideString("1234"));
  EXPECT_EQ(L"", CPDF_ToUnicodeMap::StringToWideString("<c2"));
  EXPECT_EQ(L"", CPDF_ToUnicodeMap::StringToWideString("<c2D2"));
  EXPECT_EQ(L"", CPDF_ToUnicodeMap::StringToWideString("c2ab>"));

  WideString res = L"\xc2ab";
  EXPECT_EQ(res, CPDF_ToUnicodeMap::StringToWideString("<c2ab>"));
  EXPECT_EQ(res, CPDF_ToUnicodeMap::StringToWideString("<c2abab>"));
  EXPECT_EQ(res, CPDF_ToUnicodeMap::StringToWideString("<c2ab 1234>"));

  res += L"\xfaab";
  EXPECT_EQ(res, CPDF_ToUnicodeMap::StringToWideString("<c2abFaAb>"));
  EXPECT_EQ(res, CPDF_ToUnicodeMap::StringToWideString("<c2abFaAb12>"));
}

TEST(cpdf_tounicodemap, HandleBeginBFRangeAvoidIntegerOverflow) {
  // Make sure there won't be infinite loops due to integer overflows in
  // HandleBeginBFRange().
  {
    static constexpr uint8_t kInput1[] =
        "beginbfrange<FFFFFFFF><FFFFFFFF>[<0041>]endbfrange";
    auto stream = pdfium::MakeRetain<CPDF_Stream>();
    stream->SetData(pdfium::make_span(kInput1));
    CPDF_ToUnicodeMap map(stream.Get());
    EXPECT_STREQ(L"A", map.Lookup(0xffffffff).c_str());
  }
  {
    static constexpr uint8_t kInput2[] =
        "beginbfrange<FFFFFFFF><FFFFFFFF><0042>endbfrange";
    auto stream = pdfium::MakeRetain<CPDF_Stream>();
    stream->SetData(pdfium::make_span(kInput2));
    CPDF_ToUnicodeMap map(stream.Get());
    EXPECT_STREQ(L"B", map.Lookup(0xffffffff).c_str());
  }
  {
    static constexpr uint8_t kInput3[] =
        "beginbfrange<FFFFFFFF><FFFFFFFF><00410042>endbfrange";
    auto stream = pdfium::MakeRetain<CPDF_Stream>();
    stream->SetData(pdfium::make_span(kInput3));
    CPDF_ToUnicodeMap map(stream.Get());
    EXPECT_STREQ(L"AB", map.Lookup(0xffffffff).c_str());
  }
}

TEST(cpdf_tounicodemap, InsertIntoMultimap) {
  {
    // Both the CIDs and the unicodes are different.
    static constexpr uint8_t kInput1[] =
        "beginbfchar<1><0041><2><0042>endbfchar";
    auto stream = pdfium::MakeRetain<CPDF_Stream>();
    stream->SetData(pdfium::make_span(kInput1));
    CPDF_ToUnicodeMap map(stream.Get());
    EXPECT_EQ(1u, map.ReverseLookup(0x0041));
    EXPECT_EQ(2u, map.ReverseLookup(0x0042));
    EXPECT_EQ(2u, map.GetMultimapSizeForTesting());
  }
  {
    // The same CID with different unicodes.
    static constexpr uint8_t kInput2[] =
        "beginbfrange<0><0><0041><0><0><0042>endbfrange";
    auto stream = pdfium::MakeRetain<CPDF_Stream>();
    stream->SetData(pdfium::make_span(kInput2));
    CPDF_ToUnicodeMap map(stream.Get());
    EXPECT_EQ(0u, map.ReverseLookup(0x0041));
    EXPECT_EQ(0u, map.ReverseLookup(0x0042));
    EXPECT_EQ(2u, map.GetMultimapSizeForTesting());
  }
  {
    // Duplicate mappings of CID 0 to unicode "A". There should be only 1 entry
    // in `m_Multimap`.
    static constexpr uint8_t kInput3[] =
        "beginbfrange<0><0>[<0041>]endbfrange\n"
        "beginbfchar<0><0041>endbfchar";
    auto stream = pdfium::MakeRetain<CPDF_Stream>();
    stream->SetData(pdfium::make_span(kInput3));
    CPDF_ToUnicodeMap map(stream.Get());
    EXPECT_EQ(0u, map.ReverseLookup(0x0041));
    EXPECT_EQ(1u, map.GetMultimapSizeForTesting());
  }
}
