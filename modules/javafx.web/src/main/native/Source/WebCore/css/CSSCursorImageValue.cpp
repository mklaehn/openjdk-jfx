/*
 * Copyright (C) 2006 Rob Buis <buis@kde.org>
 *           (C) 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "CSSCursorImageValue.h"

#include "CSSImageSetValue.h"
#include "CSSImageValue.h"
#include "CachedImage.h"
#include "CachedResourceLoader.h"
#include "SVGCursorElement.h"
#include "SVGLengthContext.h"
#include "SVGURIReference.h"
#include <wtf/MathExtras.h>
#include <wtf/text/StringBuilder.h>
#include <wtf/text/WTFString.h>

namespace WebCore {

CSSCursorImageValue::CSSCursorImageValue(Ref<CSSValue>&& imageValue, bool hasHotSpot, const IntPoint& hotSpot)
    : CSSValue(CursorImageClass)
    , m_imageValue(WTFMove(imageValue))
    , m_hasHotSpot(hasHotSpot)
    , m_hotSpot(hotSpot)
{
    if (is<CSSImageValue>(m_imageValue.get()))
        m_originalURL = downcast<CSSImageValue>(m_imageValue.get()).url();
}

CSSCursorImageValue::~CSSCursorImageValue()
{
    for (auto* element : m_cursorElements)
        element->removeClient(*this);
}

String CSSCursorImageValue::customCSSText() const
{
    StringBuilder result;
    result.append(m_imageValue.get().cssText());
    if (m_hasHotSpot) {
        result.append(' ');
        result.appendNumber(m_hotSpot.x());
        result.append(' ');
        result.appendNumber(m_hotSpot.y());
    }
    return result.toString();
}

SVGCursorElement* CSSCursorImageValue::updateCursorElement(const Document& document)
{
    if (!m_originalURL.hasFragmentIdentifier())
        return nullptr;

    auto* element = SVGURIReference::targetElementFromIRIString(m_originalURL, document);
    if (!is<SVGCursorElement>(element))
        return nullptr;

    auto& cursorElement = downcast<SVGCursorElement>(*element);
    if (m_cursorElements.add(&cursorElement).isNewEntry) {
        cursorElementChanged(cursorElement);
        cursorElement.addClient(*this);
    }
    return &cursorElement;
}

void CSSCursorImageValue::cursorElementRemoved(SVGCursorElement& cursorElement)
{
    m_cursorElements.remove(&cursorElement);
}

void CSSCursorImageValue::cursorElementChanged(SVGCursorElement& cursorElement)
{
    // FIXME: This will override hot spot specified in CSS, which is probably incorrect.
    SVGLengthContext lengthContext(nullptr);
    m_hasHotSpot = true;
    float x = std::round(cursorElement.x().value(lengthContext));
    m_hotSpot.setX(static_cast<int>(x));

    float y = std::round(cursorElement.y().value(lengthContext));
    m_hotSpot.setY(static_cast<int>(y));
}

std::pair<CachedImage*, float> CSSCursorImageValue::loadImage(CachedResourceLoader& loader, const ResourceLoaderOptions& options)
{
    if (is<CSSImageSetValue>(m_imageValue.get()))
        return downcast<CSSImageSetValue>(m_imageValue.get()).loadBestFitImage(loader, options);

    if (auto* cursorElement = updateCursorElement(*loader.document())) {
        if (cursorElement->href() != downcast<CSSImageValue>(m_imageValue.get()).url())
            m_imageValue = CSSImageValue::create(loader.document()->completeURL(cursorElement->href()));
    }

    return { downcast<CSSImageValue>(m_imageValue.get()).loadImage(loader, options), 1 };
}

bool CSSCursorImageValue::equals(const CSSCursorImageValue& other) const
{
    return m_hasHotSpot ? other.m_hasHotSpot && m_hotSpot == other.m_hotSpot : !other.m_hasHotSpot
        && compareCSSValue(m_imageValue, other.m_imageValue);
}

} // namespace WebCore
