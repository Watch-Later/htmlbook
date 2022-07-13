#include "globalstring.h"

#include <string>
#include <set>

namespace htmlbook {

GlobalString::GlobalString(const std::string_view& value)
    : m_value(add(value))
{
}

std::string_view GlobalString::add(const std::string_view& value)
{
    static std::set<std::string, std::less<>> table;
    auto lb = table.lower_bound(value);
    if(lb != table.end() && *lb == value)
        return *lb;
    return *table.emplace_hint(lb, value);
}

const GlobalString nullString;
const GlobalString emptyString("");
const GlobalString starString("*");

namespace namespaceuri {

const GlobalString xhtml("http://www.w3.org/1999/xhtml");
const GlobalString mathml("http://www.w3.org/1998/Math/MathML");
const GlobalString svg("http://www.w3.org/2000/svg");

} // namespace namespaceuri

namespace htmlnames {

const GlobalString aTag("a");
const GlobalString abbrTag("abbr");
const GlobalString acronymTag("acronym");
const GlobalString addressTag("address");
const GlobalString areaTag("area");
const GlobalString articleTag("article");
const GlobalString asideTag("aside");
const GlobalString audioTag("audio");
const GlobalString bTag("b");
const GlobalString baseTag("base");
const GlobalString basefontTag("basefont");
const GlobalString bgsoundTag("bgsound");
const GlobalString bigTag("big");
const GlobalString blockquoteTag("blockquote");
const GlobalString bodyTag("body");
const GlobalString brTag("br");
const GlobalString buttonTag("button");
const GlobalString canvasTag("canvas");
const GlobalString captionTag("caption");
const GlobalString centerTag("center");
const GlobalString citeTag("cite");
const GlobalString codeTag("code");
const GlobalString colTag("col");
const GlobalString colgroupTag("colgroup");
const GlobalString commandTag("command");
const GlobalString datagridTag("datagrid");
const GlobalString datalistTag("datalist");
const GlobalString dcellTag("dcell");
const GlobalString dcolTag("dcol");
const GlobalString ddTag("dd");
const GlobalString detailsTag("details");
const GlobalString delTag("del");
const GlobalString dfnTag("dfn");
const GlobalString dirTag("dir");
const GlobalString divTag("div");
const GlobalString dlTag("dl");
const GlobalString drowTag("drow");
const GlobalString dtTag("dt");
const GlobalString emTag("em");
const GlobalString embedTag("embed");
const GlobalString fieldsetTag("fieldset");
const GlobalString figcaptionTag("figcaption");
const GlobalString figureTag("figure");
const GlobalString fontTag("font");
const GlobalString footerTag("footer");
const GlobalString formTag("form");
const GlobalString frameTag("frame");
const GlobalString framesetTag("frameset");
const GlobalString h1Tag("h1");
const GlobalString h2Tag("h2");
const GlobalString h3Tag("h3");
const GlobalString h4Tag("h4");
const GlobalString h5Tag("h5");
const GlobalString h6Tag("h6");
const GlobalString headTag("head");
const GlobalString headerTag("header");
const GlobalString hgroupTag("hgroup");
const GlobalString hrTag("hr");
const GlobalString htmlTag("html");
const GlobalString iTag("i");
const GlobalString iframeTag("iframe");
const GlobalString imageTag("image");
const GlobalString imgTag("img");
const GlobalString inputTag("input");
const GlobalString insTag("ins");
const GlobalString isindexTag("isindex");
const GlobalString kbdTag("kbd");
const GlobalString keygenTag("keygen");
const GlobalString labelTag("lebel");
const GlobalString layerTag("layer");
const GlobalString legendTag("legend");
const GlobalString liTag("li");
const GlobalString linkTag("link");
const GlobalString listingTag("listing");
const GlobalString mainTag("main");
const GlobalString mapTag("map");
const GlobalString markTag("mark");
const GlobalString marqueeTag("marquee");
const GlobalString menuTag("menu");
const GlobalString metaTag("meta");
const GlobalString meterTag("meter");
const GlobalString navTag("nav");
const GlobalString nobrTag("nobr");
const GlobalString noembedTag("noembed");
const GlobalString noframesTag("noframes");
const GlobalString nolayerTag("nolayer");
const GlobalString noscriptTag("noscript");
const GlobalString objectTag("object");
const GlobalString olTag("ol");
const GlobalString optgroupTag("optgroup");
const GlobalString optionTag("option");
const GlobalString pTag("p");
const GlobalString paramTag("param");
const GlobalString plaintextTag("plaintext");
const GlobalString preTag("pre");
const GlobalString progressTag("progress");
const GlobalString qTag("q");
const GlobalString rpTag("rp");
const GlobalString rtTag("rt");
const GlobalString rubyTag("ruby");
const GlobalString sTag("s");
const GlobalString sampTag("samp");
const GlobalString scriptTag("script");
const GlobalString sectionTag("section");
const GlobalString selectTag("select");
const GlobalString smallTag("small");
const GlobalString sourceTag("source");
const GlobalString spanTag("span");
const GlobalString strikeTag("strike");
const GlobalString strongTag("strong");
const GlobalString styleTag("style");
const GlobalString subTag("sub");
const GlobalString summaryTag("summary");
const GlobalString supTag("sup");
const GlobalString tableTag("table");
const GlobalString tbodyTag("tbody");
const GlobalString tdTag("td");
const GlobalString textareaTag("textarea");
const GlobalString tfootTag("tfoot");
const GlobalString thTag("th");
const GlobalString theadTag("thead");
const GlobalString titleTag("title");
const GlobalString trTag("tr");
const GlobalString trackTag("track");
const GlobalString ttTag("tt");
const GlobalString uTag("u");
const GlobalString ulTag("ul");
const GlobalString varTag("var");
const GlobalString videoTag("video");
const GlobalString wbrTag("wbr");
const GlobalString xmpTag("xmp");

const GlobalString checkedAttr("checked");
const GlobalString classAttr("class");
const GlobalString colorAttr("color");
const GlobalString disabledAttr("disabled");
const GlobalString enabledAttr("enabled");
const GlobalString faceAttr("face");
const GlobalString hrefAttr("href");
const GlobalString idAttr("id");
const GlobalString langAttr("lang");
const GlobalString sizeAttr("size");
const GlobalString styleAttr("style");
const GlobalString typeAttr("type");

} // namespace htmlnames

namespace svgnames {

const GlobalString aTag("a");
const GlobalString circleTag("circle");
const GlobalString clipPathTag("clipPath");
const GlobalString defsTag("defs");
const GlobalString descTag("desc");
const GlobalString ellipseTag("ellipse");
const GlobalString foreignObjectTag("foreignObject");
const GlobalString gTag("g");
const GlobalString imageTag("image");
const GlobalString lineTag("line");
const GlobalString linearGradientTag("linearGradient");
const GlobalString markerTag("marker");
const GlobalString maskTag("mask");
const GlobalString metadataTag("metadata");
const GlobalString pathTag("path");
const GlobalString patternTag("pattern");
const GlobalString polygonTag("polygon");
const GlobalString polylineTag("polyline");
const GlobalString radialGradientTag("radialGradient");
const GlobalString rectTag("rect");
const GlobalString stopTag("stop");
const GlobalString styleTag("style");
const GlobalString svgTag("svg");
const GlobalString switchTag("switch");
const GlobalString symbolTag("symbol");
const GlobalString textTag("text");
const GlobalString textPathTag("textPath");
const GlobalString titleTag("title");
const GlobalString trefTag("tref");
const GlobalString tspanTag("tspan");
const GlobalString useTag("use");

const GlobalString clipAttr("clip");
const GlobalString clipPathUnitsAttr("clipPathUnits");
const GlobalString clip_pathAttr("clip-path");
const GlobalString clip_ruleAttr("clip-rule");
const GlobalString colorAttr("color");
const GlobalString cxAttr("cx");
const GlobalString cyAttr("cy");
const GlobalString dAttr("d");
const GlobalString displayAttr("display");
const GlobalString dxAttr("dx");
const GlobalString dyAttr("dy");
const GlobalString fillAttr("fill");
const GlobalString fill_opacityAttr("fill-opacity");
const GlobalString fill_ruleAttr("fill-rule");
const GlobalString font_familyAttr("font-family");
const GlobalString font_sizeAttr("font-size");
const GlobalString font_size_adjustAttr("font-size-adjust");
const GlobalString font_stretchAttr("font-stretch");
const GlobalString font_styleAttr("font-style");
const GlobalString font_variantAttr("font-variant");
const GlobalString font_weightAttr("font-weight");
const GlobalString fxAttr("fx");
const GlobalString fyAttr("fy");
const GlobalString gradientTransformAttr("gradientTransform");
const GlobalString gradientUnitsAttr("gradientUnits");
const GlobalString heightAttr("height");
const GlobalString lengthAdjustAttr("lengthAdjust");
const GlobalString letter_spacingAttr("letter-spacing");
const GlobalString markerHeightAttr("markerHeight");
const GlobalString markerUnitsAttr("markerUnits");
const GlobalString markerWidthAttr("markerWidth");
const GlobalString marker_endAttr("marker-end");
const GlobalString marker_midAttr("marker-mid");
const GlobalString marker_startAttr("marker-start");
const GlobalString maskAttr("mask");
const GlobalString maskContentUnitsAttr("maskContentUnits");
const GlobalString maskUnitsAttr("maskUnits");
const GlobalString overflowAttr("overflow");
const GlobalString overline_positionAttr("overline-position");
const GlobalString overline_thicknessAttr("overline-thickness");
const GlobalString pathAttr("path");
const GlobalString patternContentUnitsAttr("patternContentUnits");
const GlobalString patternTransformAttr("patternTransform");
const GlobalString patternUnitsAttr("patternUnits");
const GlobalString pointsAttr("points");
const GlobalString preserveAspectRatioAttr("preserveAspectRatio");
const GlobalString rAttr("r");
const GlobalString refXAttr("refX");
const GlobalString refYAttr("refY");
const GlobalString rotateAttr("rotate");
const GlobalString rxAttr("rx");
const GlobalString ryAttr("ry");
const GlobalString spacingAttr("spacing");
const GlobalString spreadMethodAttr("spreadMethod");
const GlobalString startOffsetAttr("startOffset");
const GlobalString stop_colorAttr("stop-color");
const GlobalString stop_opacityAttr("stop-opacity");
const GlobalString strikethrough_positionAttr("strikethrough-position");
const GlobalString strikethrough_thicknessAttr("strikethrough-thickness");
const GlobalString strokeAttr("stroke");
const GlobalString stroke_dasharrayAttr("stroke-dasharray");
const GlobalString stroke_dashoffsetAttr("stroke-dashoffset");
const GlobalString stroke_linecapAttr("stroke-linecap");
const GlobalString stroke_linejoinAttr("stroke-linejoin");
const GlobalString stroke_miterlimitAttr("stroke-miterlimit");
const GlobalString stroke_opacityAttr("stroke-opacity");
const GlobalString stroke_widthAttr("stroke-width");
const GlobalString styleAttr("style");
const GlobalString textLengthAttr("textLength");
const GlobalString text_anchorAttr("text-anchor");
const GlobalString text_decorationAttr("text-decoration");
const GlobalString titleAttr("title");
const GlobalString transformAttr("transform");
const GlobalString underline_positionAttr("underline-position");
const GlobalString underline_thicknessAttr("underline-thickness");
const GlobalString unicodeAttr("unicode");
const GlobalString unicode_bidiAttr("unicode-bidi");
const GlobalString unicode_rangeAttr("unicode-range");
const GlobalString viewBoxAttr("viewBox");
const GlobalString visibilityAttr("visibility");
const GlobalString widthAttr("width");
const GlobalString word_spacingAttr("word-spacing");
const GlobalString writing_modeAttr("writing-mode");
const GlobalString xAttr("x");
const GlobalString x1Attr("x1");
const GlobalString x2Attr("x2");
const GlobalString yAttr("y");
const GlobalString y1Attr("y1");
const GlobalString y2Attr("y2");

} // namespace svgnames

namespace mathnames {

const GlobalString annotation_xmlTag("annotation-xml");
const GlobalString malignmarkTag("malignmark");
const GlobalString mathTag("math");
const GlobalString mglyphTag("mglyph");
const GlobalString miTag("mi");
const GlobalString mnTag("mn");
const GlobalString moTag("mo");
const GlobalString msTag("ms");
const GlobalString mtextTag("mtext");

const GlobalString encodingAttr("encoding");

} // namespace mathnames

} // namespace htmlbook
