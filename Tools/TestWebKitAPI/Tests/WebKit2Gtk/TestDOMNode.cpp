/*
 * Copyright (C) 2013 Igalia S.L.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2,1 of the License, or (at your option) any later version.
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

#include "WebViewTest.h"
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

static void testWebKitDOMNodeHierarchyNavigation(WebViewTest* test, gconstpointer)
{
    static const char* testHTML = "<html><head><title>This is the title</title></head><body><p>1</p><p>2</p><p>3</p></body></html>";
    test->loadHtml(testHTML, 0);
    test->waitUntilLoadFinished();

    g_assert(test->runWebProcessTest("WebKitDOMNode", "hierarchy-navigation"));
}

static void testWebKitDOMNodeInsertion(WebViewTest* test, gconstpointer)
{
    static const char* testHTML = "<html><body></body></html>";
    test->loadHtml(testHTML, 0);
    test->waitUntilLoadFinished();

    g_assert(test->runWebProcessTest("WebKitDOMNode", "insertion"));
}

static void testWebKitDOMNodeTagNames(WebViewTest* test, gconstpointer)
{
    static const char* testHTML = "<html><head></head><body>"
        "<video id='video' preload='none'>"
        "    <source src='movie.ogg' type='video/ogg'>"
        "        Your browser does not support the video tag."
        "</video>"
        "<video id='video2' preload='none'>"
        "     <source src='movie.ogg' type='video/ogg'>"
        "        Your browser does not support the video tag."
        "</video>"
        "<input type='hidden' id='test' name='finish' value='false'></body></html>";
    test->loadHtml(testHTML, 0);
    test->waitUntilLoadFinished();

    g_assert(test->runWebProcessTest("WebKitDOMNode", "tag-names"));
}

void beforeAll()
{
    WebViewTest::add("WebKitDOMNode", "hierarchy-navigation", testWebKitDOMNodeHierarchyNavigation);
    WebViewTest::add("WebKitDOMNode", "insertion", testWebKitDOMNodeInsertion);
    WebViewTest::add("WebKitDOMNode", "tag-names", testWebKitDOMNodeTagNames);
}

void afterAll()
{
}
