#include <catch2/catch_test_macros.hpp>

#include <array>
#include <iostream>

#include <noarr/structures/extra/traverser.hpp>
#include <noarr/structures_extended.hpp>

using namespace noarr;

TEST_CASE("Traverser trivial", "[traverser]") {
	using at = noarr::array<'x', 20, noarr::array<'y', 30, noarr::scalar<int>>>;
	using bt = noarr::array<'y', 30, noarr::array<'z', 40, noarr::scalar<int>>>;
	using ct = noarr::array<'x', 20, noarr::array<'z', 40, noarr::scalar<int>>>;

	using xt = noarr::array<'z', 40, noarr::array<'x', 20, noarr::array<'y', 30, noarr::scalar<int>>>>;

	using u1t = union_t<at>;
	using u2t = union_t<at, bt>;
	using u3t = union_t<at, bt, ct>;

	at a;
	bt b;
	ct c;

	u1t u1;
	u2t u2;
	u3t u3;

	REQUIRE(u1.length<'x'>(state<>()) == 20);
	REQUIRE(u1.length<'y'>(state<>()) == 30);

	REQUIRE(u2.length<'x'>(state<>()) == 20);
	REQUIRE(u2.length<'y'>(state<>()) == 30);
	REQUIRE(u2.length<'z'>(state<>()) == 40);

	REQUIRE(u3.length<'x'>(state<>()) == 20);
	REQUIRE(u3.length<'y'>(state<>()) == 30);
	REQUIRE(u3.length<'z'>(state<>()) == 40);

	REQUIRE(std::is_same_v<u1t::signature, at::signature>);
	REQUIRE(std::is_same_v<u2t::signature, xt::signature>);
	REQUIRE(std::is_same_v<u3t::signature, xt::signature>);

	int i = 0;

	traverser(a, b, c).for_each([&i](auto s){
		static_assert(decltype(s)::template contains<index_in<'x'>>);
		static_assert(decltype(s)::template contains<index_in<'y'>>);
		static_assert(decltype(s)::template contains<index_in<'z'>>);

		REQUIRE(s.template get<index_in<'x'>>() < 20);
		REQUIRE(s.template get<index_in<'y'>>() < 30);
		REQUIRE(s.template get<index_in<'z'>>() < 40);

		REQUIRE(get_index<'x'>(s) == s.template get<index_in<'x'>>());
		REQUIRE(get_index<'y'>(s) == s.template get<index_in<'y'>>());
		REQUIRE(get_index<'z'>(s) == s.template get<index_in<'z'>>());

		int j =
			+ s.template get<index_in<'z'>>() * 30 * 20
			+ s.template get<index_in<'x'>>() * 30
			+ s.template get<index_in<'y'>>()
		;

		REQUIRE(i == j);

		i++;
	});

	REQUIRE(i == 20*30*40);
}

TEST_CASE("Traverser ordered", "[traverser]") {
	using at = noarr::array<'x', 20, noarr::array<'y', 30, noarr::scalar<int>>>;
	using bt = noarr::array<'y', 30, noarr::array<'z', 40, noarr::scalar<int>>>;
	using ct = noarr::array<'x', 20, noarr::array<'z', 40, noarr::scalar<int>>>;

	using xt = noarr::array<'z', 40, noarr::array<'x', 20, noarr::array<'y', 30, noarr::scalar<int>>>>;

	using u1t = union_t<at>;
	using u2t = union_t<at, bt>;
	using u3t = union_t<at, bt, ct>;

	at a;
	bt b;
	ct c;

	u1t u1;
	u2t u2;
	u3t u3;

	REQUIRE(u1.length<'x'>(state<>()) == 20);
	REQUIRE(u1.length<'y'>(state<>()) == 30);

	REQUIRE(u2.length<'x'>(state<>()) == 20);
	REQUIRE(u2.length<'y'>(state<>()) == 30);
	REQUIRE(u2.length<'z'>(state<>()) == 40);

	REQUIRE(u3.length<'x'>(state<>()) == 20);
	REQUIRE(u3.length<'y'>(state<>()) == 30);
	REQUIRE(u3.length<'z'>(state<>()) == 40);

	REQUIRE(std::is_same_v<u1t::signature, at::signature>);
	REQUIRE(std::is_same_v<u2t::signature, xt::signature>);
	REQUIRE(std::is_same_v<u3t::signature, xt::signature>);

	int i = 0;

	traverser(a, b, c).order(reorder<'x', 'y', 'z'>()).for_each([&i](auto s){
		static_assert(decltype(s)::template contains<index_in<'x'>>);
		static_assert(decltype(s)::template contains<index_in<'y'>>);
		static_assert(decltype(s)::template contains<index_in<'z'>>);

		REQUIRE(s.template get<index_in<'x'>>() < 20);
		REQUIRE(s.template get<index_in<'y'>>() < 30);
		REQUIRE(s.template get<index_in<'z'>>() < 40);

		int j =
			+ s.template get<index_in<'x'>>() * 30 * 40
			+ s.template get<index_in<'y'>>() * 40
			+ s.template get<index_in<'z'>>()
		;

		REQUIRE(i == j);

		i++;
	});

	REQUIRE(i == 20*30*40);
}

TEST_CASE("Traverser ordered renamed", "[traverser]") {
	using at = noarr::array<'x', 20, noarr::array<'y', 30, noarr::scalar<int>>>;
	using bt = noarr::array<'y', 30, noarr::array<'z', 40, noarr::scalar<int>>>;
	using ct = noarr::array<'x', 20, noarr::array<'z', 40, noarr::scalar<int>>>;

	using xt = noarr::array<'z', 40, noarr::array<'x', 20, noarr::array<'y', 30, noarr::scalar<int>>>>;

	using u1t = union_t<at>;
	using u2t = union_t<at, bt>;
	using u3t = union_t<at, bt, ct>;

	at a;
	bt b;
	ct c;

	u1t u1;
	u2t u2;
	u3t u3;

	REQUIRE(u1.length<'x'>(state<>()) == 20);
	REQUIRE(u1.length<'y'>(state<>()) == 30);

	REQUIRE(u2.length<'x'>(state<>()) == 20);
	REQUIRE(u2.length<'y'>(state<>()) == 30);
	REQUIRE(u2.length<'z'>(state<>()) == 40);

	REQUIRE(u3.length<'x'>(state<>()) == 20);
	REQUIRE(u3.length<'y'>(state<>()) == 30);
	REQUIRE(u3.length<'z'>(state<>()) == 40);

	REQUIRE(std::is_same_v<u1t::signature, at::signature>);
	REQUIRE(std::is_same_v<u2t::signature, xt::signature>);
	REQUIRE(std::is_same_v<u3t::signature, xt::signature>);

	int i = 0;

	traverser(a, b, c).order(reorder<'x', 'y', 'z'>() ^ noarr::rename<'y', 't'>()).for_each([&i](auto s){
		static_assert(decltype(s)::template contains<index_in<'x'>>);
		static_assert(decltype(s)::template contains<index_in<'y'>>);
		static_assert(decltype(s)::template contains<index_in<'z'>>);

		REQUIRE(s.template get<index_in<'x'>>() < 20);
		REQUIRE(s.template get<index_in<'y'>>() < 30);
		REQUIRE(s.template get<index_in<'z'>>() < 40);

		int j =
			+ s.template get<index_in<'x'>>() * 30 * 40
			+ s.template get<index_in<'y'>>() * 40
			+ s.template get<index_in<'z'>>()
		;

		REQUIRE(i == j);

		i++;
	});

	REQUIRE(i == 20*30*40);
}

TEST_CASE("Traverser ordered renamed access", "[traverser]") {
	using at = noarr::array<'x', 20, noarr::array<'y', 30, noarr::scalar<int>>>;
	using bt = noarr::array<'y', 30, noarr::array<'z', 40, noarr::scalar<int>>>;
	using ct = noarr::array<'x', 20, noarr::array<'z', 40, noarr::scalar<int>>>;

	int ad[20*30];
	int bd[30*40];
	int cd[20*40];

	void* ap = ad;
	void* bp = bd;
	void* cp = cd;

	at a;
	bt b;
	ct c;

	traverser(a, b, c).order(reorder<'x', 'y', 'z'>() ^ noarr::rename<'y', 't'>()).for_each([&](auto s){
		REQUIRE((a | offset(s)) / sizeof(int) == s.template get<index_in<'x'>>() * 30 + s.template get<index_in<'y'>>());
		REQUIRE((b | offset(s)) / sizeof(int) == s.template get<index_in<'y'>>() * 40 + s.template get<index_in<'z'>>());
		REQUIRE((c | offset(s)) / sizeof(int) == s.template get<index_in<'x'>>() * 40 + s.template get<index_in<'z'>>());

		REQUIRE((char*) &(a | get_at(ap, s)) == (char*) ap + (a | offset(s)));
		REQUIRE((char*) &(b | get_at(bp, s)) == (char*) bp + (b | offset(s)));
		REQUIRE((char*) &(c | get_at(cp, s)) == (char*) cp + (c | offset(s)));
	});
}

TEST_CASE("Traverser ordered renamed access blocked", "[traverser blocks]") {
	using at = noarr::array<'y', 30, noarr::array<'x', 20, noarr::scalar<int>>>;
	using bt = noarr::array<'x', 20, noarr::array<'y', 30, noarr::scalar<int>>>;

	at a;
	bt b;

	std::size_t i = 0;

	traverser(a, b).order(noarr::into_blocks<'x', 'u', 'v'>(4)).for_each([&](auto s){
		REQUIRE(!decltype(s)::template contains<index_in<'u'>> & !decltype(s)::template contains<index_in<'v'>>);
		REQUIRE( decltype(s)::template contains<index_in<'x'>> &  decltype(s)::template contains<index_in<'y'>>);

		std::size_t x = s.template get<index_in<'x'>>();
		std::size_t y = s.template get<index_in<'y'>>();
		REQUIRE(y*20 + x == i);
		i++;
	});

	REQUIRE(i == 20*30);
}

TEST_CASE("Traverser ordered renamed access strip mined", "[traverser shortcuts blocks]") {
	using at = noarr::array<'y', 30, noarr::array<'x', 20, noarr::scalar<int>>>;
	using bt = noarr::array<'x', 20, noarr::array<'y', 30, noarr::scalar<int>>>;

	at a;
	bt b;

	std::size_t i = 0;

	traverser(a, b).order(noarr::strip_mine<'x', 'u', 'v'>(4)).for_each([&](auto s){
		REQUIRE(!decltype(s)::template contains<index_in<'u'>> & !decltype(s)::template contains<index_in<'v'>>);
		REQUIRE( decltype(s)::template contains<index_in<'x'>> &  decltype(s)::template contains<index_in<'y'>>);

		std::size_t x = s.template get<index_in<'x'>>();
		std::size_t y = s.template get<index_in<'y'>>();
		std::size_t u = x / 4;
		std::size_t v = x % 4;
		REQUIRE(u*30*4 + y*4 + v == i);
		i++;
	});

	REQUIRE(i == 20*30);
}

TEST_CASE("Traverser update_index (reverse example)", "[traverser shortcuts]") {
	using at = noarr::array<'z', 40, noarr::array<'y', 30, noarr::array<'x', 20, noarr::scalar<int>>>>;

	at a;

	// verify that capture works
	int last_y = 29, last_z = 39;

	traverser(a).for_each([&](auto sa){
		auto sa_ry = update_index<'y'>(sa, [&](auto y){return last_y-y;});
		REQUIRE(sa_ry.template get<index_in<'x'>>() == sa.template get<index_in<'x'>>());
		REQUIRE(sa_ry.template get<index_in<'y'>>() == 29-sa.template get<index_in<'y'>>());
		REQUIRE(sa_ry.template get<index_in<'z'>>() == sa.template get<index_in<'z'>>());

		auto sa_rz = update_index<'z'>(sa, [&](auto z){return last_z-z;});
		REQUIRE(sa_rz.template get<index_in<'x'>>() == sa.template get<index_in<'x'>>());
		REQUIRE(sa_rz.template get<index_in<'y'>>() == sa.template get<index_in<'y'>>());
		REQUIRE(sa_rz.template get<index_in<'z'>>() == 39-sa.template get<index_in<'z'>>());
	});
}

TEST_CASE("Traverser neighbor (stencil example)", "[traverser shortcuts]") {
	using at = noarr::array<'z', 40, noarr::array<'y', 30, noarr::array<'x', 20, noarr::scalar<int>>>>;

	at a;

	int i = 0;

	traverser(a).order(slice<'x'>(1, 18) ^ slice<'y'>(1, 28)).for_each([&](auto sa){
		auto sa7 = neighbor<'y', 'x'>(sa, -1, -1);
		auto sa8 = neighbor<'y', 'x'>(sa, -1,  0);
		auto sa9 = neighbor<'y', 'x'>(sa, -1, +1);
		auto sa4 = neighbor<'y', 'x'>(sa,  0, -1);
		auto sa6 = neighbor<'y', 'x'>(sa,  0, +1);
		auto sa1 = neighbor<'y', 'x'>(sa, +1, -1);
		auto sa2 = neighbor<'y', 'x'>(sa, +1,  0);
		auto sa3 = neighbor<'y', 'x'>(sa, +1, +1);

		// does not hold generally (`neighbor` could reorder the indices in the state)
		REQUIRE(std::is_same_v<decltype(sa7), decltype(sa)>);
		REQUIRE(std::is_same_v<decltype(sa8), decltype(sa)>);
		REQUIRE(std::is_same_v<decltype(sa9), decltype(sa)>);
		REQUIRE(std::is_same_v<decltype(sa4), decltype(sa)>);
		REQUIRE(std::is_same_v<decltype(sa6), decltype(sa)>);
		REQUIRE(std::is_same_v<decltype(sa1), decltype(sa)>);
		REQUIRE(std::is_same_v<decltype(sa2), decltype(sa)>);
		REQUIRE(std::is_same_v<decltype(sa3), decltype(sa)>);

		REQUIRE(sa7.template get<index_in<'x'>>() == sa.template get<index_in<'x'>>() - 1);
		REQUIRE(sa8.template get<index_in<'x'>>() == sa.template get<index_in<'x'>>());
		REQUIRE(sa9.template get<index_in<'x'>>() == sa.template get<index_in<'x'>>() + 1);
		REQUIRE(sa4.template get<index_in<'x'>>() == sa.template get<index_in<'x'>>() - 1);
		REQUIRE(sa6.template get<index_in<'x'>>() == sa.template get<index_in<'x'>>() + 1);
		REQUIRE(sa1.template get<index_in<'x'>>() == sa.template get<index_in<'x'>>() - 1);
		REQUIRE(sa2.template get<index_in<'x'>>() == sa.template get<index_in<'x'>>());
		REQUIRE(sa3.template get<index_in<'x'>>() == sa.template get<index_in<'x'>>() + 1);

		REQUIRE(sa7.template get<index_in<'y'>>() == sa.template get<index_in<'y'>>() - 1);
		REQUIRE(sa8.template get<index_in<'y'>>() == sa.template get<index_in<'y'>>() - 1);
		REQUIRE(sa9.template get<index_in<'y'>>() == sa.template get<index_in<'y'>>() - 1);
		REQUIRE(sa4.template get<index_in<'y'>>() == sa.template get<index_in<'y'>>());
		REQUIRE(sa6.template get<index_in<'y'>>() == sa.template get<index_in<'y'>>());
		REQUIRE(sa1.template get<index_in<'y'>>() == sa.template get<index_in<'y'>>() + 1);
		REQUIRE(sa2.template get<index_in<'y'>>() == sa.template get<index_in<'y'>>() + 1);
		REQUIRE(sa3.template get<index_in<'y'>>() == sa.template get<index_in<'y'>>() + 1);

		REQUIRE(sa7.template get<index_in<'z'>>() == sa.template get<index_in<'z'>>());
		REQUIRE(sa8.template get<index_in<'z'>>() == sa.template get<index_in<'z'>>());
		REQUIRE(sa9.template get<index_in<'z'>>() == sa.template get<index_in<'z'>>());
		REQUIRE(sa4.template get<index_in<'z'>>() == sa.template get<index_in<'z'>>());
		REQUIRE(sa6.template get<index_in<'z'>>() == sa.template get<index_in<'z'>>());
		REQUIRE(sa1.template get<index_in<'z'>>() == sa.template get<index_in<'z'>>());
		REQUIRE(sa2.template get<index_in<'z'>>() == sa.template get<index_in<'z'>>());
		REQUIRE(sa3.template get<index_in<'z'>>() == sa.template get<index_in<'z'>>());

		REQUIRE(sa7.template get<index_in<'x'>>() < 18);
		REQUIRE(sa7.template get<index_in<'y'>>() < 28);
		REQUIRE(sa3.template get<index_in<'x'>>() >= 2);
		REQUIRE(sa3.template get<index_in<'y'>>() >= 2);
		REQUIRE(sa3.template get<index_in<'x'>>() < 20);
		REQUIRE(sa3.template get<index_in<'y'>>() < 30);

		i++;
	});

	REQUIRE(i == 40*28*18);
}

TEST_CASE("Traverser step in order", "[traverser shortcuts]") {
	auto a = noarr::scalar<int>() ^ noarr::array<'x', 20>();

	unsigned i = 0;

	traverser(a).order(noarr::step<'x'>(3, 5)).for_each([&](auto sa){
		auto o = a | offset(sa);
		REQUIRE(get_index<'x'>(sa) == (5*i+3));
		REQUIRE(o == (5*i+3) * sizeof(int));
		i++;
	});

	REQUIRE(i == 20/5);
}

TEST_CASE("Traverser step in structure", "[traverser shortcuts]") {
	auto a = noarr::scalar<int>() ^ noarr::array<'x', 20>() ^ noarr::step<'x'>(3, 5);

	unsigned i = 0;

	traverser(a).for_each([&](auto sa){
		auto o = a | offset(sa);
		REQUIRE(get_index<'x'>(sa) == i);
		REQUIRE(o == (5*i+3) * sizeof(int));
		i++;
	});

	REQUIRE(i == 20/5);
}

TEST_CASE("Nested traverser traditional", "[traverser]") {
	auto mat = scalar<float>() ^ array<'j', 50>() ^ array<'i', 50>();

	unsigned ei = 0;

	traverser(mat).order(reorder<'i'>()).for_each([&](auto idxs) {
		auto i = get_index<'i'>(idxs);
		REQUIRE(i == ei);
		auto sliced = mat ^ fix<'i'>(i) ^ slice<'j'>(i, 50 - i);
		unsigned ej = 0;
		traverser(sliced).order(reorder<'j'>()).for_each([&](auto idxs) {
			auto j = get_index<'j'>(idxs);
			REQUIRE(j == ej);
			auto o = sliced | offset(idxs);
			REQUIRE(o == (50*ei + ei+ej) * sizeof(float));
			ej++;
		});
		REQUIRE(ej == 50-ei);
		ei++;
	});
	REQUIRE(ei == 50);
}

TEST_CASE("Nested traverser simplified", "[traverser]") {
	auto mat = scalar<float>() ^ array<'j', 50>() ^ array<'i', 50>();

	unsigned ei = 0;

	traverser(mat).order(reorder<'i'>()).for_each([&](auto idxs) {
		auto i = get_index<'i'>(idxs);
		REQUIRE(i == ei);
		auto sliced = mat ^ fix(idxs) ^ slice<'j'>(i, 50 - i);
		unsigned ej = 0;
		traverser(sliced).for_each([&](auto idxs) {
			auto j = get_index<'j'>(idxs);
			REQUIRE(j == ej);
			auto o = sliced | offset(idxs);
			REQUIRE(o == (50*ei + ei+ej) * sizeof(float));
			ej++;
		});
		REQUIRE(ej == 50-ei);
		ei++;
	});
	REQUIRE(ei == 50);
}

TEST_CASE("Nested traverser for_dims", "[traverser]") {
	auto mat = scalar<float>() ^ array<'j', 50>() ^ array<'i', 50>();

	unsigned ei = 0;

	traverser(mat).for_dims<'i'>([&](auto trav) {
		auto i = get_index<'i'>(trav.state());
		REQUIRE(i == ei);
		unsigned ej = 0;
		trav.order(slice<'j'>(i, 50 - i)).for_each([&](auto idxs) {
			auto j = get_index<'j'>(idxs);
			REQUIRE(j-i == ej);
			auto o = mat | offset(idxs);
			REQUIRE(o == (50*ei + ei+ej) * sizeof(float));
			ej++;
		});
		REQUIRE(ej == 50-ei);
		ei++;
	});
	REQUIRE(ei == 50);
}

TEST_CASE("Traverser single state", "[traverser]") {
	auto t = traverser(scalar<float>() ^ array<'x', 100>()).order(fix<'x'>(42));

	auto s = t.state();

	REQUIRE(get_index<'x'>(s) == 42);
}

TEST_CASE("Traverser single state multi struct", "[traverser]") {
	auto t = traverser(scalar<float>() ^ array<'x', 100>() ^ array<'y', 200>(), scalar<float>() ^ array<'y', 200>() ^ array<'z', 300>()).order(fix<'x'>(42) ^ fix<'y'>(142) ^ fix<'z'>(242));

	auto s = t.state();

	REQUIRE(get_index<'x'>(s) == 42);
	REQUIRE(get_index<'y'>(s) == 142);
	REQUIRE(get_index<'z'>(s) == 242);
}
