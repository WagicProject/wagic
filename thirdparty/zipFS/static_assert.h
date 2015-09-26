
#pragma once



namespace mstatic_assert {

	template <bool> class compile_time_error;
	template <> class compile_time_error<true> { };

}



#define mstatic_assert(expr)				{ mstatic_assert::compile_time_error<((expr) != 0)> ERROR_STATIC_ASSERT; (void) ERROR_STATIC_ASSERT; }
#define mstatic_assert_msg(expr, msg)	{ mstatic_assert::compile_time_error<((expr) != 0)> ERROR_##msg; (void) ERROR_##msg; }
