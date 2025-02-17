/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "SimpleLineLayout.h"

#include "FontCache.h"
#include "Frame.h"
#include "GraphicsContext.h"
#include "HTMLTextFormControlElement.h"
#include "HitTestLocation.h"
#include "HitTestRequest.h"
#include "HitTestResult.h"
#include "InlineTextBox.h"
#include "LineWidth.h"
#include "PaintInfo.h"
#include "RenderBlockFlow.h"
#include "RenderChildIterator.h"
#include "RenderStyle.h"
#include "RenderText.h"
#include "RenderTextControl.h"
#include "RenderView.h"
#include "Settings.h"
#include "SimpleLineLayoutFlowContents.h"
#include "SimpleLineLayoutFunctions.h"
#include "Text.h"
#include "TextPaintStyle.h"

namespace WebCore {
namespace SimpleLineLayout {

template <typename CharacterType>
static bool canUseForText(const CharacterType* text, unsigned length, const SimpleFontData& fontData)
{
    // FIXME: <textarea maxlength=0> generates empty text node.
    if (!length)
        return false;
    for (unsigned i = 0; i < length; ++i) {
        UChar character = text[i];
        if (character == ' ')
            continue;

        // These would be easy to support.
        if (character == noBreakSpace)
            return false;
        if (character == softHyphen)
            return false;

        UCharDirection direction = u_charDirection(character);
        if (direction == U_RIGHT_TO_LEFT || direction == U_RIGHT_TO_LEFT_ARABIC
            || direction == U_RIGHT_TO_LEFT_EMBEDDING || direction == U_RIGHT_TO_LEFT_OVERRIDE
            || direction == U_LEFT_TO_RIGHT_EMBEDDING || direction == U_LEFT_TO_RIGHT_OVERRIDE
            || direction == U_POP_DIRECTIONAL_FORMAT || direction == U_BOUNDARY_NEUTRAL)
            return false;

        if (!fontData.glyphForCharacter(character))
            return false;
    }
    return true;
}

static bool canUseForText(const RenderText& textRenderer, const SimpleFontData& fontData)
{
    if (textRenderer.is8Bit())
        return canUseForText(textRenderer.characters8(), textRenderer.textLength(), fontData);
    return canUseForText(textRenderer.characters16(), textRenderer.textLength(), fontData);
}

bool canUseFor(const RenderBlockFlow& flow)
{
    if (!flow.frame().settings().simpleLineLayoutEnabled())
        return false;
    if (!flow.firstChild())
        return false;
    // This currently covers <blockflow>#text</blockflow> and mutiple (sibling) RenderText cases.
    // The <blockflow><inline>#text</inline></blockflow> case is also popular and should be relatively easy to cover.
    for (const auto& renderer : childrenOfType<RenderObject>(flow)) {
        if (!is<RenderText>(renderer))
            return false;
    }
    if (!flow.isHorizontalWritingMode())
        return false;
    if (flow.flowThreadState() != RenderObject::NotInsideFlowThread)
        return false;
    // Printing does pagination without a flow thread.
    if (flow.document().paginated())
        return false;
    if (flow.hasOutline())
        return false;
    if (flow.isRubyText() || flow.isRubyBase())
        return false;
    if (flow.parent()->isDeprecatedFlexibleBox())
        return false;
    // FIXME: Implementation of wrap=hard looks into lineboxes.
    if (flow.parent()->isTextArea() && flow.parent()->element()->fastHasAttribute(HTMLNames::wrapAttr))
        return false;
    // FIXME: Placeholders do something strange.
    if (is<RenderTextControl>(*flow.parent()) && downcast<RenderTextControl>(*flow.parent()).textFormControlElement().placeholderElement())
        return false;
    const RenderStyle& style = flow.style();
    if (style.textDecorationsInEffect() != TextDecorationNone)
        return false;
    if (style.textAlign() == JUSTIFY)
        return false;
    // Non-visible overflow should be pretty easy to support.
    if (style.overflowX() != OVISIBLE || style.overflowY() != OVISIBLE)
        return false;
    if (!style.textIndent().isZero())
        return false;
    if (!style.wordSpacing().isZero() || style.letterSpacing())
        return false;
    if (!style.isLeftToRightDirection())
        return false;
    if (style.lineBoxContain() != RenderStyle::initialLineBoxContain())
        return false;
    if (style.writingMode() != TopToBottomWritingMode)
        return false;
    if (style.lineBreak() != LineBreakAuto)
        return false;
    if (style.wordBreak() != NormalWordBreak)
        return false;
    if (style.unicodeBidi() != UBNormal || style.rtlOrdering() != LogicalOrder)
        return false;
    if (style.lineAlign() != LineAlignNone || style.lineSnap() != LineSnapNone)
        return false;
    if (style.hyphens() == HyphensAuto)
        return false;
    if (style.textEmphasisFill() != TextEmphasisFillFilled || style.textEmphasisMark() != TextEmphasisMarkNone)
        return false;
    if (style.textShadow())
        return false;
    if (style.textOverflow() || (flow.isAnonymousBlock() && flow.parent()->style().textOverflow()))
        return false;
    if (style.hasPseudoStyle(FIRST_LINE) || style.hasPseudoStyle(FIRST_LETTER))
        return false;
    if (style.hasTextCombine())
        return false;
    if (style.backgroundClip() == TextFillBox)
        return false;
    if (style.borderFit() == BorderFitLines)
        return false;
    if (style.lineBreak() != LineBreakAuto)
        return false;

    // We can't use the code path if any lines would need to be shifted below floats. This is because we don't keep per-line y coordinates.
    if (flow.containsFloats()) {
        float minimumWidthNeeded = std::numeric_limits<float>::max();
        for (const auto& textRenderer : childrenOfType<RenderText>(flow)) {
            minimumWidthNeeded = std::min(minimumWidthNeeded, textRenderer.minLogicalWidth());

            for (auto& floatRenderer : *flow.floatingObjectSet()) {
                ASSERT(floatRenderer);
                float availableWidth = flow.availableLogicalWidthForLine(floatRenderer->y(), false);
                if (availableWidth < minimumWidthNeeded)
                    return false;
            }
        }
    }
    if (style.font().primaryFont()->isSVGFont())
        return false;
    // We assume that all lines have metrics based purely on the primary font.
    auto& primaryFontData = *style.font().primaryFont();
    if (primaryFontData.isLoading())
        return false;
    for (const auto& textRenderer : childrenOfType<RenderText>(flow)) {
        if (textRenderer.isCombineText() || textRenderer.isCounter() || textRenderer.isQuote() || textRenderer.isTextFragment()
            || textRenderer.isSVGInlineText())
            return false;
        if (style.font().codePath(TextRun(textRenderer.text())) != Font::Simple)
            return false;
        if (!canUseForText(textRenderer, primaryFontData))
            return false;
    }
    return true;
}

static float computeLineLeft(ETextAlign textAlign, float availableWidth, float committedWidth, float logicalLeftOffset)
{
    float remainingWidth = availableWidth - committedWidth;
    float left = logicalLeftOffset;
    switch (textAlign) {
    case LEFT:
    case WEBKIT_LEFT:
    case TASTART:
        return left;
    case RIGHT:
    case WEBKIT_RIGHT:
    case TAEND:
        return left + std::max<float>(remainingWidth, 0);
    case CENTER:
    case WEBKIT_CENTER:
        return left + std::max<float>(remainingWidth / 2, 0);
    case JUSTIFY:
        ASSERT_NOT_REACHED();
        break;
    }
    ASSERT_NOT_REACHED();
    return 0;
}

struct TextFragment {
    TextFragment()
        : start(0)
        , isCollapsedWhitespace(false)
        , end(0)
        , isWhitespaceOnly(false)
        , isBreakable(false)
        , isLineBreak(false)
        , width(0)
    {
    }

    TextFragment(unsigned textStart, unsigned textEnd, float textWidth, bool isWhitespaceOnly)
        : start(textStart)
        , isCollapsedWhitespace(false)
        , end(textEnd)
        , isWhitespaceOnly(isWhitespaceOnly)
        , isBreakable(false)
        , isLineBreak(false)
        , width(textWidth)
    {
    }

    bool isEmpty() const
    {
        return start == end;
    }

    unsigned start : 31;
    bool isCollapsedWhitespace : 1;
    unsigned end : 31;
    bool isWhitespaceOnly : 1;
    bool isBreakable;
    bool isLineBreak;
    float width;
};

struct LineState {
    LineState()
        : availableWidth(0)
        , logicalLeftOffset(0)
        , lineStartRunIndex(0)
        , uncommittedStart(0)
        , uncommittedEnd(0)
        , uncommittedWidth(0)
        , committedWidth(0)
        , committedLogicalRight(0)
        , position(0)
        , trailingWhitespaceWidth(0)
    {
    }

    void commitAndCreateRun(Layout::RunVector& lineRuns)
    {
        if (uncommittedStart == uncommittedEnd)
            return;

        lineRuns.append(Run(uncommittedStart, uncommittedEnd, committedLogicalRight, committedLogicalRight + uncommittedWidth, false));
        // Move uncommitted to committed.
        committedWidth += uncommittedWidth;
        committedLogicalRight += committedWidth;

        uncommittedStart = uncommittedEnd;
        uncommittedWidth = 0;
    }

    void addUncommitted(const TextFragment& fragment)
    {
        unsigned uncomittedFragmentLength = fragment.end - uncommittedEnd;
        uncommittedWidth += fragment.width;
        uncommittedEnd = fragment.end;
        position = uncommittedEnd;
        trailingWhitespaceWidth = fragment.isWhitespaceOnly ? fragment.width : 0;
        trailingWhitespaceLength = fragment.isWhitespaceOnly ? uncomittedFragmentLength  : 0;
    }

    void addUncommittedWhitespace(float whitespaceWidth)
    {
        addUncommitted(TextFragment(uncommittedEnd, uncommittedEnd + 1, whitespaceWidth, true));
    }

    void jumpTo(unsigned newPositon, float logicalRight)
    {
        position = newPositon;

        uncommittedStart = newPositon;
        uncommittedEnd = newPositon;
        uncommittedWidth = 0;
        committedLogicalRight = logicalRight;
    }

    float width() const
    {
        return committedWidth + uncommittedWidth;
    }

    bool fits(float extra) const
    {
        return availableWidth >= width() + extra;
    }

    void removeCommittedTrailingWhitespace()
    {
        ASSERT(!uncommittedWidth);
        committedWidth -= trailingWhitespaceWidth;
        committedLogicalRight -= trailingWhitespaceWidth;
    }

    void resetTrailingWhitespace()
    {
        trailingWhitespaceWidth = 0;
        trailingWhitespaceLength = 0;
    }

    float availableWidth;
    float logicalLeftOffset;
    unsigned lineStartRunIndex; // The run that the line starts with.

    unsigned uncommittedStart;
    unsigned uncommittedEnd;
    float uncommittedWidth;
    float committedWidth;
    float committedLogicalRight; // Last committed X (coordinate) position.

    unsigned position;

    float trailingWhitespaceWidth; // Use this to remove trailing whitespace without re-mesuring the text.
    float trailingWhitespaceLength;

    TextFragment oveflowedFragment;
};

static void removeTrailingWhitespace(LineState& lineState, Layout::RunVector& lineRuns, const FlowContents& flowContents)
{
    const auto& style = flowContents.style();
    bool preWrap = style.wrapLines && !style.collapseWhitespace;
    // Trailing whitespace gets removed when we either collapse whitespace or pre-wrap is present.
    if (!(style.collapseWhitespace || preWrap)) {
        lineState.resetTrailingWhitespace();
        return;
    }

    ASSERT(lineRuns.size());
    Run& lastRun = lineRuns.last();

    unsigned lastPosition = lineState.position;
    bool trailingPreWrapWhitespaceNeedsToBeRemoved = false;
    // When pre-wrap is present, trailing whitespace needs to be removed:
    // 1. from the "next line": when at least the first charater fits. When even the first whitespace is wider that the available width,
    // we don't remove any whitespace at all.
    // 2. from this line: remove whitespace, unless it's the only fragment on the line -so removing the whitesapce would produce an empty line.
    if (preWrap) {
        if (lineState.oveflowedFragment.isWhitespaceOnly && !lineState.oveflowedFragment.isEmpty() && lineState.availableWidth >= lineState.committedWidth) {
            lineState.position = lineState.oveflowedFragment.end;
            lineState.oveflowedFragment = TextFragment();
        }
        if (lineState.trailingWhitespaceLength) {
            // Check if we've got only whitespace on this line.
            trailingPreWrapWhitespaceNeedsToBeRemoved = !(lineState.committedWidth == lineState.trailingWhitespaceWidth);
        }
    }
    if (lineState.trailingWhitespaceLength && (style.collapseWhitespace || trailingPreWrapWhitespaceNeedsToBeRemoved)) {
        lastRun.logicalRight -= lineState.trailingWhitespaceWidth;
        lastRun.end -= lineState.trailingWhitespaceLength;
        if (lastRun.start == lastRun.end)
            lineRuns.removeLast();
        lineState.removeCommittedTrailingWhitespace();
    }

    // If we skipped any whitespace and now the line end is a "preserved" newline, skip the newline too as we are wrapping the line here already.
    if (lastPosition != lineState.position && style.preserveNewline && !flowContents.isEnd(lineState.position) && flowContents.isLineBreak(lineState.position))
        ++lineState.position;
}

static void initializeNewLine(LineState& lineState, const FlowContents& flowContents, unsigned lineStartRunIndex)
{
    lineState.lineStartRunIndex = lineStartRunIndex;
    // Skip leading whitespace if collapsing whitespace, unless there's an uncommitted fragment pushed from the previous line.
    // FIXME: Be smarter when the run from the previous line does not fit the current line. Right now, we just reprocess it.
    if (lineState.oveflowedFragment.width) {
        if (lineState.fits(lineState.oveflowedFragment.width))
            lineState.addUncommitted(lineState.oveflowedFragment);
        else
            lineState.jumpTo(lineState.oveflowedFragment.start, 0); // Start over with this fragment.
    } else {
        unsigned spaceCount = 0;
        lineState.jumpTo(flowContents.style().collapseWhitespace ? flowContents.findNextNonWhitespacePosition(lineState.position, spaceCount) : lineState.position, 0);
    }
    lineState.oveflowedFragment = TextFragment();
}

static TextFragment splitFragmentToFitLine(TextFragment& fragmentToSplit, float availableWidth, bool keepAtLeastOneCharacter, const FlowContents& flowContents)
{
    // Fast path for single char fragments.
    if (fragmentToSplit.start + 1 == fragmentToSplit.end) {
        if (keepAtLeastOneCharacter)
            return TextFragment();

        TextFragment fragmentForNextLine(fragmentToSplit);
        fragmentToSplit.end = fragmentToSplit.start;
        fragmentToSplit.width = 0;
        return fragmentForNextLine;
    }
    // Simple binary search to find out what fits the current line.
    // FIXME: add surrogate pair support.
    unsigned left = fragmentToSplit.start;
    unsigned right = fragmentToSplit.end - 1; // We can ignore the last character. It surely does not fit.
    float width = 0;
    while (left < right) {
        unsigned middle = (left + right) / 2;
        width = flowContents.textWidth(fragmentToSplit.start, middle + 1, 0);
        if (availableWidth > width)
            left = middle + 1;
        else if (availableWidth < width)
            right = middle;
        else {
            right = middle + 1;
            break;
        }
    }

    if (keepAtLeastOneCharacter && right == fragmentToSplit.start)
        ++right;
    TextFragment fragmentForNextLine(fragmentToSplit);
    fragmentToSplit.end = right;
    fragmentToSplit.width = fragmentToSplit.isEmpty() ? 0 : flowContents.textWidth(fragmentToSplit.start, right, 0);

    fragmentForNextLine.start = fragmentToSplit.end;
    fragmentForNextLine.width -= fragmentToSplit.width;
    return fragmentForNextLine;
}

static TextFragment nextFragment(unsigned previousFragmentEnd, const FlowContents& flowContents, float xPosition)
{
    // A fragment can have
    // 1. new line character when preserveNewline is on (not considered as whitespace) or
    // 2. whitespace (collasped, non-collapsed multi or single) or
    // 3. non-whitespace characters.
    const auto& style = flowContents.style();
    TextFragment fragment;
    fragment.isLineBreak = flowContents.isLineBreak(previousFragmentEnd);
    unsigned spaceCount = 0;
    unsigned whitespaceEnd = previousFragmentEnd;
    if (!fragment.isLineBreak)
        whitespaceEnd = flowContents.findNextNonWhitespacePosition(previousFragmentEnd, spaceCount);
    fragment.isWhitespaceOnly = previousFragmentEnd < whitespaceEnd;
    fragment.start = previousFragmentEnd;
    if (fragment.isWhitespaceOnly)
        fragment.end = whitespaceEnd;
    else if (fragment.isLineBreak)
        fragment.end = fragment.start + 1;
    else
        fragment.end = flowContents.findNextBreakablePosition(previousFragmentEnd + 1);
    bool multiple = fragment.start + 1 < fragment.end;
    fragment.isCollapsedWhitespace = multiple && fragment.isWhitespaceOnly && style.collapseWhitespace;
    // Non-collapsed whitespace or just plain words when "break word on overflow" is on can wrap.
    fragment.isBreakable = multiple && ((fragment.isWhitespaceOnly && !fragment.isCollapsedWhitespace) || (!fragment.isWhitespaceOnly && style.breakWordOnOverflow));

    // Compute fragment width or just use the pre-computed whitespace widths.
    unsigned fragmentLength = fragment.end - fragment.start;
    if (fragment.isCollapsedWhitespace)
        fragment.width = style.spaceWidth;
    else if (fragment.isLineBreak)
        fragment.width = 0; // Newline character's width is 0.
    else if (fragmentLength == spaceCount) // Space only.
        fragment.width = style.spaceWidth * spaceCount;
    else
        fragment.width = flowContents.textWidth(fragment.start, fragment.end, xPosition);
    return fragment;
}

static bool createLineRuns(LineState& lineState, Layout::RunVector& lineRuns, const FlowContents& flowContents)
{
    const auto& style = flowContents.style();
    bool lineCanBeWrapped = style.wrapLines || style.breakWordOnOverflow;
    while (!flowContents.isEnd(lineState.position)) {
        // Find the next text fragment. Start from the end of the previous fragment -current line end.
        TextFragment fragment = nextFragment(lineState.position, flowContents, lineState.width());
        if ((lineCanBeWrapped && !lineState.fits(fragment.width)) || fragment.isLineBreak) {
            // Overflow wrapping behaviour:
            // 1. Newline character: wraps the line unless it's treated as whitespace.
            // 2. Whitesapce collapse on: whitespace is skipped.
            // 3. Whitespace collapse off: whitespace is wrapped.
            // 4. First, non-whitespace fragment is either wrapped or kept on the line. (depends on overflow-wrap)
            // 5. Non-whitespace fragment when there's already another fragment on the line gets pushed to the next line.
            bool isFirstFragment = !lineState.width();
            if (fragment.isLineBreak) {
                if (isFirstFragment)
                    lineState.addUncommitted(fragment);
                else {
                    // No need to add the new line fragment if there's already content on the line. We are about to close this line anyway.
                    ++lineState.position;
                }
            } else if (style.collapseWhitespace && fragment.isWhitespaceOnly) {
                // Whitespace collapse is on: whitespace that doesn't fit is simply skipped.
                lineState.position = fragment.end;
            } else if (fragment.isWhitespaceOnly || ((isFirstFragment && style.breakWordOnOverflow) || !style.wrapLines)) { // !style.wrapLines: bug138102(preserve existing behavior)
                // Whitespace collapse is off or non-whitespace content. split the fragment; (modified)fragment -> this lineState, oveflowedFragment -> next line.
                // When this is the only (first) fragment, the first character stays on the line, even if it does not fit.
                lineState.oveflowedFragment = splitFragmentToFitLine(fragment, lineState.availableWidth - lineState.width(), isFirstFragment, flowContents);
                if (!fragment.isEmpty()) {
                    // Whitespace fragments can get pushed entirely to the next line.
                    lineState.addUncommitted(fragment);
                }
            } else if (isFirstFragment) {
                // Non-breakable non-whitespace first fragment. Add it to the current line. -it overflows though.
                lineState.addUncommitted(fragment);
            } else {
                // Non-breakable non-whitespace fragment when there's already a fragment on the line. Push it to the next line.
                lineState.oveflowedFragment = fragment;
            }
            break;
        }
        // When the current fragment is collapsed whitespace, we need to create a run for what we've processed so far.
        if (fragment.isCollapsedWhitespace) {
            // One trailing whitespace to preserve.
            lineState.addUncommittedWhitespace(style.spaceWidth);
            lineState.commitAndCreateRun(lineRuns);
            // And skip the collapsed whitespace.
            lineState.jumpTo(fragment.end, lineState.width() + fragment.width - style.spaceWidth);
        } else
            lineState.addUncommitted(fragment);
    }
    lineState.commitAndCreateRun(lineRuns);
    return flowContents.isEnd(lineState.position) && lineState.oveflowedFragment.isEmpty();
}

static void closeLineEndingAndAdjustRuns(LineState& lineState, Layout::RunVector& lineRuns, unsigned& lineCount, const FlowContents& flowContents)
{
    if (lineState.lineStartRunIndex == lineRuns.size())
        return;

    ASSERT(lineRuns.size());
    removeTrailingWhitespace(lineState, lineRuns, flowContents);
    // Adjust runs' position by taking line's alignment into account.
    if (float lineLogicalLeft = computeLineLeft(flowContents.style().textAlign, lineState.availableWidth, lineState.committedWidth, lineState.logicalLeftOffset)) {
        for (unsigned i = lineState.lineStartRunIndex; i < lineRuns.size(); ++i) {
            lineRuns[i].logicalLeft += lineLogicalLeft;
            lineRuns[i].logicalRight += lineLogicalLeft;
        }
    }
    lineRuns.last().isEndOfLine = true;
    lineState.committedWidth = 0;
    lineState.committedLogicalRight = 0;
    ++lineCount;
}

static void splitRunsAtRendererBoundary(Layout::RunVector& lineRuns, const FlowContents& flowContents)
{
    // FIXME: We should probably split during run construction instead of as a separate pass.
    if (lineRuns.isEmpty())
        return;
    if (flowContents.hasOneSegment())
        return;

    unsigned runIndex = 0;
    do {
        const Run& run = lineRuns.at(runIndex);
        ASSERT(run.start != run.end);
        auto& startSegment = flowContents.segmentForPosition(run.start);
        if (run.end <= startSegment.end)
            continue;
        // This run overlaps multiple renderers. Split it up.
        // Split run at the renderer's boundary and create a new run for the left side, while use the current run as the right side.
        float logicalRightOfLeftRun = run.logicalLeft + flowContents.textWidth(run.start, startSegment.end, run.logicalLeft);
        lineRuns.insert(runIndex, Run(run.start, startSegment.end, run.logicalLeft, logicalRightOfLeftRun, false));
        Run& rightSideRun = lineRuns.at(runIndex + 1);
        rightSideRun.start = startSegment.end;
        rightSideRun.logicalLeft = logicalRightOfLeftRun;
    } while (++runIndex < lineRuns.size());
}

static void updateLineConstrains(const RenderBlockFlow& flow, float& availableWidth, float& logicalLeftOffset)
{
    LayoutUnit height = flow.logicalHeight();
    LayoutUnit logicalHeight = flow.minLineHeightForReplacedRenderer(false, 0);
    float logicalRightOffset = flow.logicalRightOffsetForLine(height, false, logicalHeight);
    logicalLeftOffset = flow.logicalLeftOffsetForLine(height, false, logicalHeight);
    availableWidth = std::max<float>(0, logicalRightOffset - logicalLeftOffset);
}

static void createTextRuns(Layout::RunVector& runs, RenderBlockFlow& flow, unsigned& lineCount)
{
    LayoutUnit borderAndPaddingBefore = flow.borderAndPaddingBefore();
    LayoutUnit lineHeight = lineHeightFromFlow(flow);
    LineState lineState;
    bool isEndOfContent = false;
    FlowContents flowContents = FlowContents(flow);

    do {
        flow.setLogicalHeight(lineHeight * lineCount + borderAndPaddingBefore);
        updateLineConstrains(flow, lineState.availableWidth, lineState.logicalLeftOffset);
        initializeNewLine(lineState, flowContents, runs.size());
        isEndOfContent = createLineRuns(lineState, runs, flowContents);
        closeLineEndingAndAdjustRuns(lineState, runs, lineCount, flowContents);
    } while (!isEndOfContent);

    splitRunsAtRendererBoundary(runs, flowContents);
    ASSERT(!lineState.uncommittedWidth);
}

std::unique_ptr<Layout> create(RenderBlockFlow& flow)
{
    unsigned lineCount = 0;
    Layout::RunVector runs;

    createTextRuns(runs, flow, lineCount);
    for (auto& renderer : childrenOfType<RenderObject>(flow)) {
        ASSERT(is<RenderText>(renderer));
        renderer.clearNeedsLayout();
    }
    return Layout::create(runs, lineCount);
}

std::unique_ptr<Layout> Layout::create(const RunVector& runVector, unsigned lineCount)
{
    void* slot = WTF::fastMalloc(sizeof(Layout) + sizeof(Run) * runVector.size());
    return std::unique_ptr<Layout>(new (NotNull, slot) Layout(runVector, lineCount));
}

Layout::Layout(const RunVector& runVector, unsigned lineCount)
    : m_lineCount(lineCount)
    , m_runCount(runVector.size())
{
    memcpy(m_runs, runVector.data(), m_runCount * sizeof(Run));
}

}
}
