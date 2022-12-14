#ifndef GLOBALSTRING_H
#define GLOBALSTRING_H

#include "heapstring.h"

namespace htmlbook {

class GlobalString {
public:
    GlobalString() = default;
    GlobalString(const std::string_view& value);

    const char* data() const { return m_value.data(); }
    size_t length() const { return m_value.length(); }
    const char& at(size_t index) const { return m_value.at(index); }
    const HeapString& value() const { return m_value; }
    bool empty() const { return m_value.empty(); }
    operator const std::string_view&() const { return m_value; }
    operator const HeapString&() const { return m_value; }

private:
    static HeapString add(const std::string_view& value);
    HeapString m_value;
};

inline std::ostream& operator<<(std::ostream& o, const GlobalString& in) { return o << in.value(); }

inline bool operator==(const GlobalString& a, const GlobalString& b) { return a.data() == b.data(); }
inline bool operator!=(const GlobalString& a, const GlobalString& b) { return a.data() != b.data(); }

inline bool operator==(const GlobalString& a, const std::string_view& b) { return a.value() == b; }
inline bool operator!=(const GlobalString& a, const std::string_view& b) { return a.value() != b; }

inline bool operator==(const std::string_view& a, const GlobalString& b) { return a == b.value(); }
inline bool operator!=(const std::string_view& a, const GlobalString& b) { return a != b.value(); }

inline bool operator<(const GlobalString& a, const GlobalString& b) { return a.value() < b.value(); }
inline bool operator>(const GlobalString& a, const GlobalString& b) { return a.value() > b.value(); }

inline bool operator<(const GlobalString& a, const std::string_view& b) { return a.value() < b; }
inline bool operator>(const GlobalString& a, const std::string_view& b) { return a.value() > b; }

inline bool operator<(const std::string_view& a, const GlobalString& b) { return a < b.value(); }
inline bool operator>(const std::string_view& a, const GlobalString& b) { return a > b.value(); }

using GlobalStringList = std::vector<GlobalString>;

extern const std::string emptyString;

extern const GlobalString nullGlo;
extern const GlobalString emptyGlo;
extern const GlobalString starGlo;

namespace namespaceuri {

extern const GlobalString xhtml;
extern const GlobalString mathml;
extern const GlobalString svg;

} // namespace namespaceuri

extern const GlobalString aTag;
extern const GlobalString abbrTag;
extern const GlobalString addressTag;
extern const GlobalString appletTag;
extern const GlobalString areaTag;
extern const GlobalString articleTag;
extern const GlobalString asideTag;
extern const GlobalString bTag;
extern const GlobalString baseTag;
extern const GlobalString basefontTag;
extern const GlobalString bgsoundTag;
extern const GlobalString bigTag;
extern const GlobalString blockquoteTag;
extern const GlobalString bodyTag;
extern const GlobalString brTag;
extern const GlobalString buttonTag;
extern const GlobalString captionTag;
extern const GlobalString centerTag;
extern const GlobalString codeTag;
extern const GlobalString colTag;
extern const GlobalString colgroupTag;
extern const GlobalString commandTag;
extern const GlobalString ddTag;
extern const GlobalString detailsTag;
extern const GlobalString dirTag;
extern const GlobalString divTag;
extern const GlobalString dlTag;
extern const GlobalString dtTag;
extern const GlobalString emTag;
extern const GlobalString embedTag;
extern const GlobalString fieldsetTag;
extern const GlobalString figcaptionTag;
extern const GlobalString figureTag;
extern const GlobalString fontTag;
extern const GlobalString footerTag;
extern const GlobalString formTag;
extern const GlobalString frameTag;
extern const GlobalString framesetTag;
extern const GlobalString h1Tag;
extern const GlobalString h2Tag;
extern const GlobalString h3Tag;
extern const GlobalString h4Tag;
extern const GlobalString h5Tag;
extern const GlobalString h6Tag;
extern const GlobalString headTag;
extern const GlobalString headerTag;
extern const GlobalString hgroupTag;
extern const GlobalString hrTag;
extern const GlobalString htmlTag;
extern const GlobalString iTag;
extern const GlobalString iframeTag;
extern const GlobalString imageTag;
extern const GlobalString imgTag;
extern const GlobalString inputTag;
extern const GlobalString keygenTag;
extern const GlobalString liTag;
extern const GlobalString linkTag;
extern const GlobalString listingTag;
extern const GlobalString mainTag;
extern const GlobalString mapTag;
extern const GlobalString markTag;
extern const GlobalString marqueeTag;
extern const GlobalString menuTag;
extern const GlobalString metaTag;
extern const GlobalString meterTag;
extern const GlobalString navTag;
extern const GlobalString nobrTag;
extern const GlobalString noembedTag;
extern const GlobalString noframesTag;
extern const GlobalString nolayerTag;
extern const GlobalString noscriptTag;
extern const GlobalString objectTag;
extern const GlobalString olTag;
extern const GlobalString optgroupTag;
extern const GlobalString optionTag;
extern const GlobalString pTag;
extern const GlobalString paramTag;
extern const GlobalString plaintextTag;
extern const GlobalString preTag;
extern const GlobalString progressTag;
extern const GlobalString qTag;
extern const GlobalString rpTag;
extern const GlobalString rtTag;
extern const GlobalString rubyTag;
extern const GlobalString sTag;
extern const GlobalString sampTag;
extern const GlobalString scriptTag;
extern const GlobalString sectionTag;
extern const GlobalString selectTag;
extern const GlobalString smallTag;
extern const GlobalString sourceTag;
extern const GlobalString spanTag;
extern const GlobalString strikeTag;
extern const GlobalString strongTag;
extern const GlobalString styleTag;
extern const GlobalString subTag;
extern const GlobalString summaryTag;
extern const GlobalString supTag;
extern const GlobalString tableTag;
extern const GlobalString tbodyTag;
extern const GlobalString tdTag;
extern const GlobalString textareaTag;
extern const GlobalString tfootTag;
extern const GlobalString thTag;
extern const GlobalString theadTag;
extern const GlobalString titleTag;
extern const GlobalString trTag;
extern const GlobalString trackTag;
extern const GlobalString ttTag;
extern const GlobalString uTag;
extern const GlobalString ulTag;
extern const GlobalString varTag;
extern const GlobalString videoTag;
extern const GlobalString wbrTag;
extern const GlobalString xmpTag;

extern const GlobalString aTag;
extern const GlobalString circleTag;
extern const GlobalString clipPathTag;
extern const GlobalString defsTag;
extern const GlobalString descTag;
extern const GlobalString ellipseTag;
extern const GlobalString foreignObjectTag;
extern const GlobalString gTag;
extern const GlobalString imageTag;
extern const GlobalString lineTag;
extern const GlobalString linearGradientTag;
extern const GlobalString markerTag;
extern const GlobalString maskTag;
extern const GlobalString metadataTag;
extern const GlobalString pathTag;
extern const GlobalString patternTag;
extern const GlobalString polygonTag;
extern const GlobalString polylineTag;
extern const GlobalString radialGradientTag;
extern const GlobalString rectTag;
extern const GlobalString stopTag;
extern const GlobalString styleTag;
extern const GlobalString svgTag;
extern const GlobalString switchTag;
extern const GlobalString symbolTag;
extern const GlobalString textTag;
extern const GlobalString textPathTag;
extern const GlobalString titleTag;
extern const GlobalString trefTag;
extern const GlobalString tspanTag;
extern const GlobalString useTag;

extern const GlobalString annotation_xmlTag;
extern const GlobalString malignmarkTag;
extern const GlobalString mathTag;
extern const GlobalString mglyphTag;
extern const GlobalString miTag;
extern const GlobalString mnTag;
extern const GlobalString moTag;
extern const GlobalString msTag;
extern const GlobalString mtextTag;

extern const GlobalString relAttr;
extern const GlobalString cellspacingAttr;
extern const GlobalString bordercolorAttr;
extern const GlobalString textAttr;
extern const GlobalString backgroundAttr;
extern const GlobalString bgcolorAttr;
extern const GlobalString vspaceAttr;
extern const GlobalString hspaceAttr;
extern const GlobalString valignAttr;
extern const GlobalString valueAttr;
extern const GlobalString startAttr;
extern const GlobalString borderAttr;
extern const GlobalString altAttr;
extern const GlobalString checkedAttr;
extern const GlobalString classAttr;
extern const GlobalString colorAttr;
extern const GlobalString disabledAttr;
extern const GlobalString enabledAttr;
extern const GlobalString faceAttr;
extern const GlobalString heightAttr;
extern const GlobalString hrefAttr;
extern const GlobalString idAttr;
extern const GlobalString langAttr;
extern const GlobalString sizeAttr;
extern const GlobalString srcAttr;
extern const GlobalString styleAttr;
extern const GlobalString typeAttr;
extern const GlobalString widthAttr;

extern const GlobalString clipAttr;
extern const GlobalString clipPathUnitsAttr;
extern const GlobalString clip_pathAttr;
extern const GlobalString clip_ruleAttr;
extern const GlobalString colorAttr;
extern const GlobalString cxAttr;
extern const GlobalString cyAttr;
extern const GlobalString dAttr;
extern const GlobalString displayAttr;
extern const GlobalString dxAttr;
extern const GlobalString dyAttr;
extern const GlobalString fillAttr;
extern const GlobalString fill_opacityAttr;
extern const GlobalString fill_ruleAttr;
extern const GlobalString font_familyAttr;
extern const GlobalString font_sizeAttr;
extern const GlobalString font_styleAttr;
extern const GlobalString font_variantAttr;
extern const GlobalString font_weightAttr;
extern const GlobalString fxAttr;
extern const GlobalString fyAttr;
extern const GlobalString gradientTransformAttr;
extern const GlobalString gradientUnitsAttr;
extern const GlobalString heightAttr;
extern const GlobalString lengthAdjustAttr;
extern const GlobalString letter_spacingAttr;
extern const GlobalString markerHeightAttr;
extern const GlobalString markerUnitsAttr;
extern const GlobalString markerWidthAttr;
extern const GlobalString marker_endAttr;
extern const GlobalString marker_midAttr;
extern const GlobalString marker_startAttr;
extern const GlobalString maskAttr;
extern const GlobalString maskContentUnitsAttr;
extern const GlobalString maskUnitsAttr;
extern const GlobalString overflowAttr;
extern const GlobalString overline_positionAttr;
extern const GlobalString overline_thicknessAttr;
extern const GlobalString pathAttr;
extern const GlobalString patternContentUnitsAttr;
extern const GlobalString patternTransformAttr;
extern const GlobalString patternUnitsAttr;
extern const GlobalString pointsAttr;
extern const GlobalString preserveAspectRatioAttr;
extern const GlobalString rAttr;
extern const GlobalString refXAttr;
extern const GlobalString refYAttr;
extern const GlobalString rotateAttr;
extern const GlobalString rxAttr;
extern const GlobalString ryAttr;
extern const GlobalString spacingAttr;
extern const GlobalString spreadMethodAttr;
extern const GlobalString startOffsetAttr;
extern const GlobalString stop_colorAttr;
extern const GlobalString stop_opacityAttr;
extern const GlobalString strikethrough_positionAttr;
extern const GlobalString strikethrough_thicknessAttr;
extern const GlobalString strokeAttr;
extern const GlobalString stroke_dasharrayAttr;
extern const GlobalString stroke_dashoffsetAttr;
extern const GlobalString stroke_linecapAttr;
extern const GlobalString stroke_linejoinAttr;
extern const GlobalString stroke_miterlimitAttr;
extern const GlobalString stroke_opacityAttr;
extern const GlobalString stroke_widthAttr;
extern const GlobalString styleAttr;
extern const GlobalString textLengthAttr;
extern const GlobalString text_anchorAttr;
extern const GlobalString text_decorationAttr;
extern const GlobalString titleAttr;
extern const GlobalString transformAttr;
extern const GlobalString underline_positionAttr;
extern const GlobalString underline_thicknessAttr;
extern const GlobalString unicodeAttr;
extern const GlobalString unicode_bidiAttr;
extern const GlobalString unicode_rangeAttr;
extern const GlobalString viewBoxAttr;
extern const GlobalString visibilityAttr;
extern const GlobalString widthAttr;
extern const GlobalString word_spacingAttr;
extern const GlobalString writing_modeAttr;
extern const GlobalString xAttr;
extern const GlobalString x1Attr;
extern const GlobalString x2Attr;
extern const GlobalString yAttr;
extern const GlobalString y1Attr;
extern const GlobalString y2Attr;

extern const GlobalString encodingAttr;

} // namespace htmlbook

#endif // GLOBALSTRING_H
