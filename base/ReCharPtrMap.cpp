/*
 * ReCharPtrMap.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */

#include "base/rebase.hpp"

/** @class ReKeyCharPtr ReCharPtrMap.hpp "base/ReCharPtrMap.hpp"
 *
 * @brief Allows using <code>char*</code> pointers as key in <code>QMap</code>.
 *
 * The template <code>QMap</code> uses the operator < to search in the map.
 * But the <code>char*</code> pointers have no such an operator.
 * The solution is the class <code>RplMapCharPtr</code> which implements
 * this operator.
 *
 * <code>strcmp()</code> is used to implement the '<' operator.
 */

/**
 * @brief Constructor.
 *
 * @param ptr   the key used in the map.
 *              @note the pointer will be stored in the map as a key,
 *              not the content
 */
ReKeyCharPtr::ReKeyCharPtr(const char* ptr) :
   m_ptr(ptr) {
}

/** @class ReCharPtrMap ReCharPtrMap.hpp "base/ReCharPtrMap.hpp"
 *
 * @brief A template for a map using const char* as keys.
 *
 * The value type is dynamic (a parameter type of the template).
 *
 * <b>Usage:</b>
 * <pre><code>
 * ReCharPtrMap<int> ids;
 * if (! id.contains("jonny"))
 *    ids["jonny"] = 1;
 * </code></pre>
 *
 * <b>Important</b>:<br>
 * Keys used with this class must be unchangable and must live during the
 * whole life of the map.
 *
 * <b>Wrong example:</b>
 * <pre><code>
 * ReCharPtrMap<int> ids;
 * void init(int keyNo, int value){
 *    char key[10];
 *    qsnprintf(buffer, sizeof buffer, "key%d", keyNo);
 *    ids[key] = value;
 * }
 * init(1, 100);
 * </code></pre>
 * The lifetime of the key is the body of the function <code>init()</code>.
 * The key becomes invalid after the call.
 */
