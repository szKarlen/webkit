/*
 *  This file is part of the WebKit open source project.
 *  This file has been generated by generate-bindings.pl. DO NOT MODIFY!
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "WebKitDOMTestCallback.h"

#include "CSSImportRule.h"
#include "DOMObjectCache.h"
#include "Document.h"
#include "ExceptionCode.h"
#include "ExceptionCodeDescription.h"
#include "JSMainThreadExecState.h"
#include "SerializedScriptValue.h"
#include "WebKitDOMDOMStringListPrivate.h"
#include "WebKitDOMFloat32ArrayPrivate.h"
#include "WebKitDOMPrivate.h"
#include "WebKitDOMTestCallbackPrivate.h"
#include "WebKitDOMTestNodePrivate.h"
#include "gobject/ConvertToUTF8String.h"
#include <wtf/GetPtr.h>
#include <wtf/RefPtr.h>

#define WEBKIT_DOM_TEST_CALLBACK_GET_PRIVATE(obj) G_TYPE_INSTANCE_GET_PRIVATE(obj, WEBKIT_DOM_TYPE_TEST_CALLBACK, WebKitDOMTestCallbackPrivate)

typedef struct _WebKitDOMTestCallbackPrivate {
#if ENABLE(SQL_DATABASE)
    RefPtr<WebCore::TestCallback> coreObject;
#endif // ENABLE(SQL_DATABASE)
} WebKitDOMTestCallbackPrivate;

#if ENABLE(SQL_DATABASE)

namespace WebKit {

WebKitDOMTestCallback* kit(WebCore::TestCallback* obj)
{
    if (!obj)
        return 0;

    if (gpointer ret = DOMObjectCache::get(obj))
        return WEBKIT_DOM_TEST_CALLBACK(ret);

    return wrapTestCallback(obj);
}

WebCore::TestCallback* core(WebKitDOMTestCallback* request)
{
    return request ? static_cast<WebCore::TestCallback*>(WEBKIT_DOM_OBJECT(request)->coreObject) : 0;
}

WebKitDOMTestCallback* wrapTestCallback(WebCore::TestCallback* coreObject)
{
    ASSERT(coreObject);
    return WEBKIT_DOM_TEST_CALLBACK(g_object_new(WEBKIT_DOM_TYPE_TEST_CALLBACK, "core-object", coreObject, nullptr));
}

} // namespace WebKit

#endif // ENABLE(SQL_DATABASE)

G_DEFINE_TYPE(WebKitDOMTestCallback, webkit_dom_test_callback, WEBKIT_DOM_TYPE_OBJECT)

static void webkit_dom_test_callback_finalize(GObject* object)
{
    WebKitDOMTestCallbackPrivate* priv = WEBKIT_DOM_TEST_CALLBACK_GET_PRIVATE(object);
#if ENABLE(SQL_DATABASE)
    WebKit::DOMObjectCache::forget(priv->coreObject.get());
#endif // ENABLE(SQL_DATABASE)
    priv->~WebKitDOMTestCallbackPrivate();
    G_OBJECT_CLASS(webkit_dom_test_callback_parent_class)->finalize(object);
}

static GObject* webkit_dom_test_callback_constructor(GType type, guint constructPropertiesCount, GObjectConstructParam* constructProperties)
{
    GObject* object = G_OBJECT_CLASS(webkit_dom_test_callback_parent_class)->constructor(type, constructPropertiesCount, constructProperties);
#if ENABLE(SQL_DATABASE)
    WebKitDOMTestCallbackPrivate* priv = WEBKIT_DOM_TEST_CALLBACK_GET_PRIVATE(object);
    priv->coreObject = static_cast<WebCore::TestCallback*>(WEBKIT_DOM_OBJECT(object)->coreObject);
    WebKit::DOMObjectCache::put(priv->coreObject.get(), object);
#endif // ENABLE(SQL_DATABASE)
    return object;
}

static void webkit_dom_test_callback_class_init(WebKitDOMTestCallbackClass* requestClass)
{
    GObjectClass* gobjectClass = G_OBJECT_CLASS(requestClass);
    g_type_class_add_private(gobjectClass, sizeof(WebKitDOMTestCallbackPrivate));
    gobjectClass->constructor = webkit_dom_test_callback_constructor;
    gobjectClass->finalize = webkit_dom_test_callback_finalize;
}

static void webkit_dom_test_callback_init(WebKitDOMTestCallback* request)
{
    WebKitDOMTestCallbackPrivate* priv = WEBKIT_DOM_TEST_CALLBACK_GET_PRIVATE(request);
    new (priv) WebKitDOMTestCallbackPrivate();
}

gboolean webkit_dom_test_callback_callback_with_no_param(WebKitDOMTestCallback* self)
{
#if ENABLE(SQL_DATABASE)
    WebCore::JSMainThreadNullState state;
    g_return_val_if_fail(WEBKIT_DOM_IS_TEST_CALLBACK(self), FALSE);
    WebCore::TestCallback* item = WebKit::core(self);
    gboolean result = item->callbackWithNoParam();
    return result;
#else
    UNUSED_PARAM(self);
    WEBKIT_WARN_FEATURE_NOT_PRESENT("SQL Database")
    return static_cast<gboolean>(0);
#endif /* ENABLE(SQL_DATABASE) */
}

gboolean webkit_dom_test_callback_callback_with_array_param(WebKitDOMTestCallback* self, WebKitDOMFloat32Array* arrayParam)
{
#if ENABLE(SQL_DATABASE)
    WebCore::JSMainThreadNullState state;
    g_return_val_if_fail(WEBKIT_DOM_IS_TEST_CALLBACK(self), FALSE);
    g_return_val_if_fail(WEBKIT_DOM_IS_FLOAT32ARRAY(arrayParam), FALSE);
    WebCore::TestCallback* item = WebKit::core(self);
    WebCore::Float32Array* convertedArrayParam = WebKit::core(arrayParam);
    gboolean result = item->callbackWithArrayParam(convertedArrayParam);
    return result;
#else
    UNUSED_PARAM(self);
    UNUSED_PARAM(arrayParam);
    WEBKIT_WARN_FEATURE_NOT_PRESENT("SQL Database")
    return static_cast<gboolean>(0);
#endif /* ENABLE(SQL_DATABASE) */
}

gboolean webkit_dom_test_callback_callback_with_serialized_script_value_param(WebKitDOMTestCallback* self, const gchar* srzParam, const gchar* strArg)
{
#if ENABLE(SQL_DATABASE)
    WebCore::JSMainThreadNullState state;
    g_return_val_if_fail(WEBKIT_DOM_IS_TEST_CALLBACK(self), FALSE);
    g_return_val_if_fail(srzParam, FALSE);
    g_return_val_if_fail(strArg, FALSE);
    WebCore::TestCallback* item = WebKit::core(self);
    WTF::String convertedStrArg = WTF::String::fromUTF8(strArg);
    gboolean result = item->callbackWithSerializedScriptValueParam(WebCore::SerializedScriptValue::create(WTF::String::fromUTF8(srzParam)), convertedStrArg);
    return result;
#else
    UNUSED_PARAM(self);
    UNUSED_PARAM(srzParam);
    UNUSED_PARAM(strArg);
    WEBKIT_WARN_FEATURE_NOT_PRESENT("SQL Database")
    return static_cast<gboolean>(0);
#endif /* ENABLE(SQL_DATABASE) */
}

glong webkit_dom_test_callback_callback_with_non_bool_return_type(WebKitDOMTestCallback* self, const gchar* strArg)
{
#if ENABLE(SQL_DATABASE)
    WebCore::JSMainThreadNullState state;
    g_return_val_if_fail(WEBKIT_DOM_IS_TEST_CALLBACK(self), 0);
    g_return_val_if_fail(strArg, 0);
    WebCore::TestCallback* item = WebKit::core(self);
    WTF::String convertedStrArg = WTF::String::fromUTF8(strArg);
    glong result = item->callbackWithNonBoolReturnType(convertedStrArg);
    return result;
#else
    UNUSED_PARAM(self);
    UNUSED_PARAM(strArg);
    WEBKIT_WARN_FEATURE_NOT_PRESENT("SQL Database")
    return static_cast<glong>(0);
#endif /* ENABLE(SQL_DATABASE) */
}

gboolean webkit_dom_test_callback_callback_with_string_list(WebKitDOMTestCallback* self, WebKitDOMDOMStringList* listParam)
{
#if ENABLE(SQL_DATABASE)
    WebCore::JSMainThreadNullState state;
    g_return_val_if_fail(WEBKIT_DOM_IS_TEST_CALLBACK(self), FALSE);
    g_return_val_if_fail(WEBKIT_DOM_IS_DOM_STRING_LIST(listParam), FALSE);
    WebCore::TestCallback* item = WebKit::core(self);
    WebCore::DOMStringList* convertedListParam = WebKit::core(listParam);
    gboolean result = item->callbackWithStringList(convertedListParam);
    return result;
#else
    UNUSED_PARAM(self);
    UNUSED_PARAM(listParam);
    WEBKIT_WARN_FEATURE_NOT_PRESENT("SQL Database")
    return static_cast<gboolean>(0);
#endif /* ENABLE(SQL_DATABASE) */
}

gboolean webkit_dom_test_callback_callback_with_boolean(WebKitDOMTestCallback* self, gboolean boolParam)
{
#if ENABLE(SQL_DATABASE)
    WebCore::JSMainThreadNullState state;
    g_return_val_if_fail(WEBKIT_DOM_IS_TEST_CALLBACK(self), FALSE);
    WebCore::TestCallback* item = WebKit::core(self);
    gboolean result = item->callbackWithBoolean(boolParam);
    return result;
#else
    UNUSED_PARAM(self);
    UNUSED_PARAM(boolParam);
    WEBKIT_WARN_FEATURE_NOT_PRESENT("SQL Database")
    return static_cast<gboolean>(0);
#endif /* ENABLE(SQL_DATABASE) */
}

gboolean webkit_dom_test_callback_callback_requires_this_to_pass(WebKitDOMTestCallback* self, glong longParam, WebKitDOMTestNode* testNodeParam)
{
#if ENABLE(SQL_DATABASE)
    WebCore::JSMainThreadNullState state;
    g_return_val_if_fail(WEBKIT_DOM_IS_TEST_CALLBACK(self), FALSE);
    g_return_val_if_fail(WEBKIT_DOM_IS_TEST_NODE(testNodeParam), FALSE);
    WebCore::TestCallback* item = WebKit::core(self);
    WebCore::TestNode* convertedTestNodeParam = WebKit::core(testNodeParam);
    gboolean result = item->callbackRequiresThisToPass(longParam, convertedTestNodeParam);
    return result;
#else
    UNUSED_PARAM(self);
    UNUSED_PARAM(longParam);
    UNUSED_PARAM(testNodeParam);
    WEBKIT_WARN_FEATURE_NOT_PRESENT("SQL Database")
    return static_cast<gboolean>(0);
#endif /* ENABLE(SQL_DATABASE) */
}

