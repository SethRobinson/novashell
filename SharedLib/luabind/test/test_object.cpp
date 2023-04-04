// Copyright (c) 2003 Daniel Wallin and Arvid Norberg

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
// ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
// SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.


#include "test.hpp"
#include <luabind/luabind.hpp>
#include <luabind/adopt_policy.hpp>
#include <luabind/detail/debug.hpp>
#include <luabind/error.hpp>
#include <luabind/operator.hpp>

#include <boost/lexical_cast.hpp>

#include <utility>

using namespace luabind;

int test_object_param(const object& table)
{
	LUABIND_CHECK_STACK(table.interpreter());

	object current_object;
	current_object = table;
	lua_State* L = table.interpreter();

	if (type(table) == LUA_TTABLE)
	{
		int sum1 = 0;
		for (iterator i(table), e; i != e; ++i)
		{
			assert(type(*i) == LUA_TNUMBER);
			sum1 += object_cast<int>(*i);
		}

		int sum2 = 0;
		for (raw_iterator i(table), e; i != e; ++i)
		{
			assert(type(*i) == LUA_TNUMBER);
			sum2 += object_cast<int>(*i);
		}

		// test iteration of empty table
		object empty_table = newtable(L);
		for (iterator i(empty_table), e; i != e; ++i)
		{}
		
		table["sum1"] = sum1;
		table["sum2"] = sum2;
		table["blurp"] = 5;
		return 0;
	}
	else
	{
		if (type(table) != LUA_TNIL)
		{
			return 1;
		}
		else
		{
			return 2;
		}
	}
}

int test_fun()
{
	return 42;
}

struct test_param : counted_type<test_param>
{
	luabind::object obj;
	luabind::object obj2;

	bool operator==(test_param const& rhs) const
	{ return obj == rhs.obj && obj2 == rhs.obj2; }
};

COUNTER_GUARD(test_param);

int test_match(const luabind::object& o)
{
	return 0;
}

int test_match(int i)
{
	return 1;
}

void test_match_object(
	luabind::object p1
  , luabind::object p2
  , luabind::object p3)
{
	p1["ret"] = 1;
	p2["ret"] = 2;
	p3["ret"] = 3;
}

void test_main(lua_State* L)
{
	using namespace luabind;

	module(L)
	[
		def("test_object_param", &test_object_param),
		def("test_fun", &test_fun),
		def("test_match", (int(*)(const luabind::object&))&test_match),
		def("test_match", (int(*)(int))&test_match),
		def("test_match_object", &test_match_object),
	
		class_<test_param>("test_param")
			.def(constructor<>())
			.def_readwrite("obj", &test_param::obj)
			.def_readonly("obj2", &test_param::obj2)
			.def(const_self == const_self)
	];

	object uninitialized;
	TEST_CHECK(!uninitialized);
	TEST_CHECK(!uninitialized.is_valid());

	test_param temp_object;
	globals(L)["temp"] = temp_object;
	TEST_CHECK(object_cast<test_param>(globals(L)["temp"]) == temp_object);
	globals(L)["temp"] = &temp_object;
	TEST_CHECK(object_cast<test_param const*>(globals(L)["temp"]) == &temp_object);
	TEST_CHECK(globals(L)["temp"] == temp_object);
	
	DOSTRING(L,
		"t = 2\n"
		"assert(test_object_param(t) == 1)");

	DOSTRING(L, "assert(test_object_param(nil) == 2)");
	DOSTRING(L, "t = { ['oh'] = 4, 3, 5, 7, 13 }");
	DOSTRING(L, "assert(test_object_param(t) == 0)");
	DOSTRING(L, "assert(t.sum1 == 4 + 3 + 5 + 7 + 13)");
	DOSTRING(L, "assert(t.sum2 == 4 + 3 + 5 + 7 + 13)");
	DOSTRING(L, "assert(t.blurp == 5)");

	object g = globals(L);
	object ret = g["test_fun"]();
	TEST_CHECK(object_cast<int>(ret) == 42);

	DOSTRING(L, "function test_param_policies(x, y) end");
	object test_param_policies = g["test_param_policies"];
	int a = type(test_param_policies);
	TEST_CHECK(a == LUA_TFUNCTION);

	luabind::object obj;
	obj = luabind::object();

	// call the function and tell lua to adopt the pointer passed as first argument
	test_param_policies(5, new test_param())[adopt(_2)];

	DOSTRING(L, "assert(test_match(7) == 1)");
	DOSTRING(L, "assert(test_match('oo') == 0)");

	DOSTRING(L,
		"t = test_param()\n"
		"t.obj = 'foo'\n"
		"assert(t.obj == 'foo')\n"
		"assert(t.obj2 == nil)");

	DOSTRING(L,
		"function test_object_policies(a) glob = a\n"
		"return 6\n"
		"end");
	object test_object_policies = g["test_object_policies"];
	object ret_val = test_object_policies("teststring")[detail::null_type()];
	TEST_CHECK(object_cast<int>(ret_val) == 6);
	TEST_CHECK(ret_val == 6);
	TEST_CHECK(6 == ret_val);
	g["temp_val"] = 6;
	TEST_CHECK(ret_val == g["temp_val"]);
	object temp_val = g["temp_val"];
	TEST_CHECK(ret_val == temp_val);

	g["temp"] = "test string";
	TEST_CHECK(boost::lexical_cast<std::string>(g["temp"]) == "test string");
	g["temp"] = 6;
	TEST_CHECK(boost::lexical_cast<std::string>(g["temp"]) == "6");

	TEST_CHECK(object_cast<std::string>(g["glob"]) == "teststring");
	TEST_CHECK(object_cast<std::string>(gettable(g, "glob")) == "teststring");
	TEST_CHECK(object_cast<std::string>(rawget(g, "glob")) == "teststring");

	object t = newtable(L);
	TEST_CHECK(iterator(t) == iterator());
	TEST_CHECK(raw_iterator(t) == raw_iterator());

	DOSTRING(L,
		"p1 = {}\n"
		"p2 = {}\n"
		"p3 = {}\n"
		"test_match_object(p1, p2, p3)\n"
		"assert(p1.ret == 1)\n"
		"assert(p2.ret == 2)\n"
		"assert(p3.ret == 3)\n");


#ifndef LUABIND_NO_EXCEPTIONS

	try
	{
		object not_initialized;
		int i = object_cast<int>(not_initialized);
		(void)i;
		TEST_ERROR("invalid cast succeeded");
	}
	catch(luabind::cast_failed&) {}

#endif

    object not_initialized;
    TEST_CHECK(!object_cast_nothrow<int>(not_initialized));
	 TEST_CHECK(!not_initialized.is_valid());
	 TEST_CHECK(!not_initialized);

    DOSTRING(L, "t = { {1}, {2}, {3}, {4} }");

    int inner_sum = 0;

    for (iterator i(globals(L)["t"]), e; i != e; ++i)
    {
        inner_sum += object_cast<int>((*i)[1]);
    }

    TEST_CHECK(inner_sum == 1 + 2 + 3 + 4);

    DOSTRING(L, "t = { {1, 2}, {3, 4}, {5, 6}, {7, 8} }");

    inner_sum = 0;

    for (iterator i(globals(L)["t"]), e; i != e; ++i)
    {
        for (iterator j(*i), e; j != e; ++j)
        {
            inner_sum += object_cast<int>(*j);
        }
    }

    TEST_CHECK(inner_sum == 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8);
	TEST_CHECK(object_cast<int>(globals(L)["t"][2][2]) == 4);
}
