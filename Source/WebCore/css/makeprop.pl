#! /usr/bin/perl
#
#   This file is part of the WebKit project
#
#   Copyright (C) 1999 Waldo Bastian (bastian@kde.org)
#   Copyright (C) 2007, 2008, 2012, 2014 Apple Inc. All rights reserved.
#   Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
#   Copyright (C) 2010 Andras Becsi (abecsi@inf.u-szeged.hu), University of Szeged
#   Copyright (C) 2013 Google Inc. All rights reserved.
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Library General Public
#   License as published by the Free Software Foundation; either
#   version 2 of the License, or (at your option) any later version.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Library General Public License for more details.
#
#   You should have received a copy of the GNU Library General Public License
#   along with this library; see the file COPYING.LIB.  If not, write to
#   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
#   Boston, MA 02110-1301, USA.
use Getopt::Long;
use preprocessor;
use strict;
use warnings;

my $defines;
my $preprocessor;
GetOptions('defines=s' => \$defines,
           'preprocessor=s' => \$preprocessor);

my @NAMES = applyPreprocessor("CSSPropertyNames.in", $defines, $preprocessor);
die "We've reached more than 1024 CSS properties, please make sure to update CSSProperty/StylePropertyMetadata accordingly" if (scalar(@NAMES) > 1024);

my %namesHash;
my @duplicates = ();

my $numPredefinedProperties = 1;
my @names = ();
my %nameIsInherited;
my %propertiesWithStyleBuilderOptions;
my %styleBuilderOptions = (
  AutoFunctions => 1, # Defined in Source/WebCore/css/StyleBuilderConverter.h
  Converter => 1,
  Custom => 1,
  Getter => 1,
  Initial => 1,
  NameForMethods => 1,
  NoDefaultColor => 1,
  Setter => 1,
  TypeName => 1,
  VisitedLinkColorSupport => 1,
);
my %nameToId;
my @aliases = ();
foreach (@NAMES) {
  next if (m/(^\s*$)/);
  next if (/^#/);

  # Input may use a different EOL sequence than $/, so avoid chomp.
  $_ =~ s/\s*\[(.+?)\]\r?$//;
  my @options = ();
  if ($1) {
    @options = split(/\s*,\s*/, $1);
  }

  $_ =~ s/[\r\n]+$//g;
  if (exists $namesHash{$_}) {
    push @duplicates, $_;
  } else {
    $namesHash{$_} = 1;
  }
  if ($_ =~ /=/) {
    if (@options) {
        die "Options are specified on an alias $_: ", join(", ", @options) . "\n";
    }
    push @aliases, $_;
  } else {
    $nameIsInherited{$_} = 0;
    my $isUsingLegacyStyleBuilder = 0;
    $propertiesWithStyleBuilderOptions{$_} = {};
    foreach my $option (@options) {
      my ($optionName, $optionValue) = split(/=/, $option);
      if ($optionName eq "Inherited") {
        $nameIsInherited{$_} = 1;
      } elsif ($optionName eq "LegacyStyleBuilder") {
        # FIXME: This is temporary. Eventually, all properties will use the new
        # style builder and this option will go away.
        $isUsingLegacyStyleBuilder = 1;
        delete $propertiesWithStyleBuilderOptions{$_};
      } elsif ($styleBuilderOptions{$optionName}) {
        die "\"" . $optionName . "\" option was used with \"LegacyStyleBuilder\" option for " . $_ . " property." if $isUsingLegacyStyleBuilder;
        $propertiesWithStyleBuilderOptions{$_}{$optionName} = $optionValue;
      } else {
        die "Unrecognized \"" . $optionName . "\" option for " . $_ . " property.";
      }
    }

    my $id = $_;
    $id =~ s/(^[^-])|-(.)/uc($1||$2)/ge;
    $nameToId{$_} = $id;

    push @names, $_;
  }
}

if (@duplicates > 0) {
    die 'Duplicate CSS property names: ', join(', ', @duplicates) . "\n";
}

open GPERF, ">CSSPropertyNames.gperf" || die "Could not open CSSPropertyNames.gperf for writing";
print GPERF << "EOF";
%{
/* This file is automatically generated from CSSPropertyNames.in by makeprop, do not edit */
#include "config.h"
#include \"CSSProperty.h\"
#include \"CSSPropertyNames.h\"
#include \"HashTools.h\"
#include <string.h>

#include <wtf/ASCIICType.h>
#include <wtf/text/AtomicString.h>
#include <wtf/text/WTFString.h>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored \"-Wunknown-pragmas\"
#pragma clang diagnostic ignored \"-Wdeprecated-register\"
#pragma clang diagnostic ignored \"-Wimplicit-fallthrough\"
#endif

namespace WebCore {
EOF

print GPERF "const char* const propertyNameStrings[numCSSProperties] = {\n";
foreach my $name (@names) {
  print GPERF "    \"$name\",\n";
}
print GPERF "};\n\n";

print GPERF << "EOF";
%}
%struct-type
struct Property;
%omit-struct-type
%language=C++
%readonly-tables
%global-table
%compare-strncmp
%define class-name CSSPropertyNamesHash
%define lookup-function-name findPropertyImpl
%define hash-function-name propery_hash_function
%define word-array-name property_wordlist
%enum
%%
EOF

foreach my $name (@names) {
  print GPERF $name . ", CSSProperty" . $nameToId{$name} . "\n";
}

foreach my $alias (@aliases) {
  $alias =~ /^([^\s]*)[\s]*=[\s]*([^\s]*)/;
  my $name = $1;
  print GPERF $name . ", CSSProperty" . $nameToId{$2} . "\n";
}

print GPERF<< "EOF";
%%
const Property* findProperty(const char* str, unsigned int len)
{
    return CSSPropertyNamesHash::findPropertyImpl(str, len);
}

const char* getPropertyName(CSSPropertyID id)
{
    if (id < firstCSSProperty)
        return 0;
    int index = id - firstCSSProperty;
    if (index >= numCSSProperties)
        return 0;
    return propertyNameStrings[index];
}

const AtomicString& getPropertyNameAtomicString(CSSPropertyID id)
{
    if (id < firstCSSProperty)
        return nullAtom;
    int index = id - firstCSSProperty;
    if (index >= numCSSProperties)
        return nullAtom;

    static AtomicString* propertyStrings = new AtomicString[numCSSProperties]; // Intentionally never destroyed.
    AtomicString& propertyString = propertyStrings[index];
    if (propertyString.isNull()) {
        const char* propertyName = propertyNameStrings[index];
        propertyString = AtomicString(propertyName, strlen(propertyName), AtomicString::ConstructFromLiteral);
    }
    return propertyString;
}

String getPropertyNameString(CSSPropertyID id)
{
    // We share the StringImpl with the AtomicStrings.
    return getPropertyNameAtomicString(id).string();
}

String getJSPropertyName(CSSPropertyID id)
{
    char result[maxCSSPropertyNameLength + 1];
    const char* cssPropertyName = getPropertyName(id);
    const char* propertyNamePointer = cssPropertyName;
    if (!propertyNamePointer)
        return emptyString();

    char* resultPointer = result;
    while (char character = *propertyNamePointer++) {
        if (character == '-') {
            char nextCharacter = *propertyNamePointer++;
            if (!nextCharacter)
                break;
            character = (propertyNamePointer - 2 != cssPropertyName) ? toASCIIUpper(nextCharacter) : nextCharacter;
        }
        *resultPointer++ = character;
    }
    *resultPointer = '\\0';
    return WTF::String(result);
}

static const bool isInheritedPropertyTable[numCSSProperties + $numPredefinedProperties] = {
    false, // CSSPropertyInvalid
EOF

foreach my $name (@names) {
  my $id = $nameToId{$name};
  my $value = $nameIsInherited{$name} ? "true " : "false";
  print GPERF "    $value, // CSSProperty$id\n";
}

print GPERF<< "EOF";
};

bool CSSProperty::isInheritedProperty(CSSPropertyID id)
{
    ASSERT(id >= 0 && id <= lastCSSProperty);
    ASSERT(id != CSSPropertyInvalid);
    return isInheritedPropertyTable[id];
}

} // namespace WebCore

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
EOF

open HEADER, ">CSSPropertyNames.h" || die "Could not open CSSPropertyNames.h for writing";
print HEADER << "EOF";
/* This file is automatically generated from CSSPropertyNames.in by makeprop, do not edit */

#ifndef CSSPropertyNames_h
#define CSSPropertyNames_h

#include <string.h>
#include <wtf/HashFunctions.h>
#include <wtf/HashTraits.h>

namespace WTF {
class AtomicString;
class String;
}

namespace WebCore {

enum CSSPropertyID {
    CSSPropertyInvalid = 0,
EOF

my $first = $numPredefinedProperties;
my $i = $numPredefinedProperties;
my $maxLen = 0;
foreach my $name (@names) {
  print HEADER "    CSSProperty" . $nameToId{$name} . " = " . $i . ",\n";
  $i = $i + 1;
  if (length($name) > $maxLen) {
    $maxLen = length($name);
  }
}
my $num = $i - $first;
my $last = $i - 1;

print HEADER "};\n\n";
print HEADER "const int firstCSSProperty = $first;\n";
print HEADER "const int numCSSProperties = $num;\n";
print HEADER "const int lastCSSProperty = $last;\n";
print HEADER "const size_t maxCSSPropertyNameLength = $maxLen;\n";

print HEADER << "EOF";

const char* getPropertyName(CSSPropertyID);
const WTF::AtomicString& getPropertyNameAtomicString(CSSPropertyID id);
WTF::String getPropertyNameString(CSSPropertyID id);
WTF::String getJSPropertyName(CSSPropertyID);

inline CSSPropertyID convertToCSSPropertyID(int value)
{
    ASSERT((value >= firstCSSProperty && value <= lastCSSProperty) || value == CSSPropertyInvalid);
    return static_cast<CSSPropertyID>(value);
}

} // namespace WebCore

namespace WTF {
template<> struct DefaultHash<WebCore::CSSPropertyID> { typedef IntHash<unsigned> Hash; };
template<> struct HashTraits<WebCore::CSSPropertyID> : GenericHashTraits<WebCore::CSSPropertyID> {
    static const bool emptyValueIsZero = true;
    static const bool needsDestruction = false;
    static void constructDeletedValue(WebCore::CSSPropertyID& slot) { slot = static_cast<WebCore::CSSPropertyID>(WebCore::lastCSSProperty + 1); }
    static bool isDeletedValue(WebCore::CSSPropertyID value) { return value == (WebCore::lastCSSProperty + 1); }
};
}

#endif // CSSPropertyNames_h

EOF

close HEADER;

#
# StyleBuilder.cpp generator.
#

sub getScopeForFunction {
  my $name = shift;
  my $builderFunction = shift;

  return $propertiesWithStyleBuilderOptions{$name}{"Custom"}{$builderFunction} ? "StyleBuilderCustom" : "StyleBuilderFunctions";
}

sub getNameForMethods {
  my $name = shift;

  my $nameForMethods = $nameToId{$name};
  $nameForMethods =~ s/Webkit//g;
  if (exists($propertiesWithStyleBuilderOptions{$name}{"NameForMethods"})) {
    $nameForMethods = $propertiesWithStyleBuilderOptions{$name}{"NameForMethods"};
  }
  return $nameForMethods;
}

sub getAutoGetter {
  my $name = shift;
  my $renderStyle = shift;

  return $renderStyle . "->hasAuto" . getNameForMethods($name) . "()";
}

sub getAutoSetter {
  my $name = shift;
  my $renderStyle = shift;

  return $renderStyle . "->setHasAuto" . getNameForMethods($name) . "()";
}

sub getVisitedLinkSetter {
  my $name = shift;
  my $renderStyle = shift;

  return $renderStyle . "->setVisitedLink" . getNameForMethods($name);
}

foreach my $name (@names) {
  # Skip properties still using the legacy style builder.
  next unless exists($propertiesWithStyleBuilderOptions{$name});

  my $nameForMethods = getNameForMethods($name);
  $nameForMethods =~ s/Webkit//g;
  if (exists($propertiesWithStyleBuilderOptions{$name}{"NameForMethods"})) {
    $nameForMethods = $propertiesWithStyleBuilderOptions{$name}{"NameForMethods"};
  }

  if (!exists($propertiesWithStyleBuilderOptions{$name}{"TypeName"})) {
    $propertiesWithStyleBuilderOptions{$name}{"TypeName"} = "E" . $nameForMethods;
  }
  if (!exists($propertiesWithStyleBuilderOptions{$name}{"Getter"})) {
    $propertiesWithStyleBuilderOptions{$name}{"Getter"} = lcfirst($nameForMethods);
  }
  if (!exists($propertiesWithStyleBuilderOptions{$name}{"Setter"})) {
    $propertiesWithStyleBuilderOptions{$name}{"Setter"} = "set" . $nameForMethods;
  }
  if (!exists($propertiesWithStyleBuilderOptions{$name}{"Initial"})) {
    $propertiesWithStyleBuilderOptions{$name}{"Initial"} = "initial" . $nameForMethods;
  }
  if (!exists($propertiesWithStyleBuilderOptions{$name}{"Custom"})) {
    $propertiesWithStyleBuilderOptions{$name}{"Custom"} = "";
  } elsif ($propertiesWithStyleBuilderOptions{$name}{"Custom"} eq "All") {
    $propertiesWithStyleBuilderOptions{$name}{"Custom"} = "Initial|Inherit|Value";
  }
  my %customValues = map { $_ => 1 } split(/\|/, $propertiesWithStyleBuilderOptions{$name}{"Custom"});
  $propertiesWithStyleBuilderOptions{$name}{"Custom"} = \%customValues;
}

use constant {
  NOT_FOR_VISITED_LINK => 0,
  FOR_VISITED_LINK => 1,
};

sub colorFromPrimitiveValue {
  my $primitiveValue = shift;
  my $forVisitedLink = @_ ? shift : NOT_FOR_VISITED_LINK;

  return "styleResolver.colorFromPrimitiveValue(&" . $primitiveValue . ", /* forVisitedLink */ " . ($forVisitedLink ? "true" : "false") . ")";
}

use constant {
  VALUE_IS_COLOR => 0,
  VALUE_IS_PRIMITIVE => 1,
};

sub generateColorValueSetter {
  my $name = shift;
  my $value = shift;
  my $indent = shift;
  my $valueIsPrimitive = @_ ? shift : VALUE_IS_COLOR;

  my $style = "styleResolver.style()";
  my $setterContent .= $indent . "if (styleResolver.applyPropertyToRegularStyle())\n";
  my $setValue = $style . "->" . $propertiesWithStyleBuilderOptions{$name}{"Setter"};
  my $color = $valueIsPrimitive ? colorFromPrimitiveValue($value) : $value;
  $setterContent .= $indent . "    " . $setValue . "(" . $color . ");\n";
  $setterContent .= $indent . "if (styleResolver.applyPropertyToVisitedLinkStyle())\n";
  $color = $valueIsPrimitive ? colorFromPrimitiveValue($value, FOR_VISITED_LINK) : $value;
  $setterContent .= $indent . "    " . getVisitedLinkSetter($name, $style) . "(" . $color . ");\n";

  return $setterContent;
}

sub handleCurrentColorValue {
  my $name = shift;
  my $primitiveValue = shift;
  my $indent = shift;

  my $code = $indent . "if (" . $primitiveValue . ".getValueID() == CSSValueCurrentcolor) {\n";
  $code .= $indent . "    applyInherit" . $nameToId{$name} . "(styleResolver);\n";
  $code .= $indent . "    return;\n";
  $code .= $indent . "}\n";
  return $code;
}

sub generateInitialValueSetter {
  my $name = shift;
  my $indent = shift;

  my $setterContent = "";
  $setterContent .= $indent . "static void applyInitial" . $nameToId{$name} . "(StyleResolver& styleResolver)\n";
  $setterContent .= $indent . "{\n";
  my $style = "styleResolver.style()";
  if (exists $propertiesWithStyleBuilderOptions{$name}{"AutoFunctions"}) {
    $setterContent .= $indent . "    " . getAutoSetter($name, $style) . ";\n";
  } elsif (exists $propertiesWithStyleBuilderOptions{$name}{"VisitedLinkColorSupport"}) {
      my $initialColor = "RenderStyle::" . $propertiesWithStyleBuilderOptions{$name}{"Initial"} . "()";
      $setterContent .= generateColorValueSetter($name, $initialColor, $indent . "    ");
  } else {
    my $setValue = $style . "->" . $propertiesWithStyleBuilderOptions{$name}{"Setter"};
    $setterContent .= $indent . "    " . $setValue . "(RenderStyle::" . $propertiesWithStyleBuilderOptions{$name}{"Initial"} . "());\n";
  }
  $setterContent .= $indent . "}\n";

  return $setterContent;
}

sub generateInheritValueSetter {
  my $name = shift;
  my $indent = shift;

  my $setterContent = "";
  $setterContent .= $indent . "static void applyInherit" . $nameToId{$name} . "(StyleResolver& styleResolver)\n";
  $setterContent .= $indent . "{\n";
  my $parentStyle = "styleResolver.parentStyle()";
  my $style = "styleResolver.style()";
  my $setValue = $style . "->" . $propertiesWithStyleBuilderOptions{$name}{"Setter"};
  my $didCallSetValue = 0;
  if (exists $propertiesWithStyleBuilderOptions{$name}{"AutoFunctions"}) {
    $setterContent .= $indent . "    if (" . getAutoGetter($name, $parentStyle) . ") {\n";
    $setterContent .= $indent . "        " . getAutoSetter($name, $style) . ";\n";
    $setterContent .= $indent . "        return;\n";
    $setterContent .= $indent . "    }\n";
  } elsif (exists $propertiesWithStyleBuilderOptions{$name}{"VisitedLinkColorSupport"}) {
    $setterContent .= $indent . "    Color color = " . $parentStyle . "->" . $propertiesWithStyleBuilderOptions{$name}{"Getter"} . "();\n";
    if (!exists($propertiesWithStyleBuilderOptions{$name}{"NoDefaultColor"})) {
      $setterContent .= $indent . "    if (!color.isValid())\n";
      $setterContent .= $indent . "        color = " . $parentStyle . "->color();\n";
    }
    $setterContent .= generateColorValueSetter($name, "color", $indent . "    ");
    $didCallSetValue = 1;
  }
  if (!$didCallSetValue) {
    my $inheritedValue = $parentStyle . "->" .  $propertiesWithStyleBuilderOptions{$name}{"Getter"} . "()";
    $setterContent .= $indent . "    " . $setValue . "(" . $inheritedValue . ");\n";
  }
  $setterContent .= $indent . "}\n";

  return $setterContent;
}

sub generateValueSetter {
  my $name = shift;
  my $indent = shift;

  my $setterContent = "";
  $setterContent .= $indent . "static void applyValue" . $nameToId{$name} . "(StyleResolver& styleResolver, CSSValue& value)\n";
  $setterContent .= $indent . "{\n";
  my $convertedValue;
  if (exists($propertiesWithStyleBuilderOptions{$name}{"Converter"})) {
    $convertedValue = "StyleBuilderConverter::convert" . $propertiesWithStyleBuilderOptions{$name}{"Converter"} . "(styleResolver, value)";
  } else {
    $convertedValue = "static_cast<" . $propertiesWithStyleBuilderOptions{$name}{"TypeName"} . ">(downcast<CSSPrimitiveValue>(value))";
  }

  my $style = "styleResolver.style()";
  my $didCallSetValue = 0;
  if (exists $propertiesWithStyleBuilderOptions{$name}{"AutoFunctions"}) {
    $setterContent .= $indent . "    if (downcast<CSSPrimitiveValue>(value).getValueID() == CSSValueAuto) {\n";
    $setterContent .= $indent . "        ". getAutoSetter($name, $style) . ";\n";
    $setterContent .= $indent . "        return;\n";
    $setterContent .= $indent . "    }\n";
  } elsif (exists $propertiesWithStyleBuilderOptions{$name}{"VisitedLinkColorSupport"}) {
      $setterContent .= $indent . "    auto& primitiveValue = downcast<CSSPrimitiveValue>(value);\n";
      if ($name eq "color") {
        # The "color" property supports "currentColor" value. We should add a parameter.
        $setterContent .= handleCurrentColorValue($name, "primitiveValue", $indent . "    ");
      }
      $setterContent .= generateColorValueSetter($name, "primitiveValue", $indent . "    ", VALUE_IS_PRIMITIVE);
      $didCallSetValue = 1;
  }
  if (!$didCallSetValue) {
    my $setValue = $style . "->" . $propertiesWithStyleBuilderOptions{$name}{"Setter"};
    $setterContent .= $indent . "    " . $setValue . "(" . $convertedValue . ");\n";
  }
  $setterContent .= $indent . "}\n";

  return $setterContent;
}

open STYLEBUILDER, ">StyleBuilder.cpp" || die "Could not open StyleBuilder.cpp for writing";
print STYLEBUILDER << "EOF";
/* This file is automatically generated from CSSPropertyNames.in by makeprop, do not edit */

#include "config.h"
#include "StyleBuilder.h"

#include "CSSPrimitiveValueMappings.h"
#include "CSSProperty.h"
#include "RenderStyle.h"
#include "StyleBuilderConverter.h"
#include "StyleBuilderCustom.h"
#include "StyleResolver.h"

namespace WebCore {

class StyleBuilderFunctions {
public:
EOF

foreach my $name (@names) {
  # Skip properties still using the legacy style builder.
  next unless exists($propertiesWithStyleBuilderOptions{$name});

  my $indent = "    ";
  if (!$propertiesWithStyleBuilderOptions{$name}{"Custom"}{"Initial"}) {
    print STYLEBUILDER generateInitialValueSetter($name, $indent);
  }
  if (!$propertiesWithStyleBuilderOptions{$name}{"Custom"}{"Inherit"}) {
    print STYLEBUILDER generateInheritValueSetter($name, $indent);
  }
  if (!$propertiesWithStyleBuilderOptions{$name}{"Custom"}{"Value"}) {
    print STYLEBUILDER generateValueSetter($name, $indent);
  }
}

print STYLEBUILDER << "EOF";
};

bool StyleBuilder::applyProperty(CSSPropertyID property, StyleResolver& styleResolver, CSSValue& value, bool isInitial, bool isInherit)
{
    switch (property) {
EOF

foreach my $name (@names) {
  # Skip properties still using the legacy style builder.
  next unless exists($propertiesWithStyleBuilderOptions{$name});

  print STYLEBUILDER "    case CSSProperty" . $nameToId{$name} . ":\n";
  print STYLEBUILDER "        if (isInitial)\n";
  print STYLEBUILDER "            " . getScopeForFunction($name, "Initial") . "::applyInitial" . $nameToId{$name} . "(styleResolver);\n";
  print STYLEBUILDER "        else if (isInherit)\n";
  print STYLEBUILDER "            " . getScopeForFunction($name, "Inherit") . "::applyInherit" . $nameToId{$name} . "(styleResolver);\n";
  print STYLEBUILDER "        else\n";
  print STYLEBUILDER "            " . getScopeForFunction($name, "Value") . "::applyValue" . $nameToId{$name} . "(styleResolver, value);\n";
  print STYLEBUILDER "        return true;\n";
}

print STYLEBUILDER << "EOF";
        default:
            return false;
    };
}

} // namespace WebCore
EOF

close STYLEBUILDER;

my $gperf = $ENV{GPERF} ? $ENV{GPERF} : "gperf";
system("\"$gperf\" --key-positions=\"*\" -D -n -s 2 CSSPropertyNames.gperf --output-file=CSSPropertyNames.cpp") == 0 || die "calling gperf failed: $?";
