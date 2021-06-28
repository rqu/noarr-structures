#include <catch2/catch.hpp>
//#include "noarr/structures.hpp"

#include <iostream>
#include <array>
#include <utility>

#include "noarr/structures/structs.hpp"
#include "noarr/structures/funcs.hpp"
#include "noarr/structures/io.hpp"
#include "noarr/structures/struct_traits.hpp"
#include "noarr/structures/wrapper.hpp"
#include "noarr/structures/bag.hpp"

#include <tuple>

enum MatrixDataLayout { Rows = 0, Columns = 1, Zcurve = 2 };

template<MatrixDataLayout layout>
struct GetMatrixStructreStructure;

template<>
struct GetMatrixStructreStructure<MatrixDataLayout::Rows>
{
	static constexpr auto GetMatrixStructure()
	{
		return noarr::vector<'x', noarr::vector<'y', noarr::scalar<int>>>();
	}
};

template<>
struct GetMatrixStructreStructure<MatrixDataLayout::Columns>
{
	static constexpr auto GetMatrixStructure()
	{
		return noarr::vector<'y', noarr::vector<'x', noarr::scalar<int>>>();
	}
};

template<>
struct GetMatrixStructreStructure<MatrixDataLayout::Zcurve>
{
	static constexpr auto GetMatrixStructure()
	{
		return noarr::vector<'x', noarr::vector<'y', noarr::scalar<int>>>();
	}
};

template<typename Structure1, typename Structure2>
void matrix_copy(noarr::bag<Structure1>& matrix1, noarr::bag<Structure2>& matrix2)
{
	int x_size = matrix1.layout().template get_length<'x'>();
	int y_size = matrix1.layout().template get_length<'y'>();

	REQUIRE(x_size == matrix2.layout().template get_length<'x'>());
	REQUIRE(y_size == matrix2.layout().template get_length<'y'>());

	for (int i = 0; i < x_size; i++)
		for (int j = 0; j < y_size; j++)
		{
			int& value2 = matrix2.layout().template get_at<'x', 'y'>(matrix2.data(), i, j);
			value2 = matrix1.layout().template get_at<'x', 'y'>(matrix1.data(), i, j);
		}
}

template<typename Structure>
void matrix_transpose(noarr::bag<Structure>& matrix1)
{
	int x_size = matrix1.layout().template get_length<'x'>();
	int y_size = matrix1.layout().template get_length<'y'>();

	REQUIRE(x_size == y_size);

	for (int i = 0; i < x_size; i++)
		for (int j = i; j < y_size; j++)
		{
			int& value1 = matrix1.layout().template get_at<'x', 'y'>(matrix1.data(), i, j);
			int& value2 = matrix1.layout().template get_at<'x', 'y'>(matrix1.data(), j, i);
			std::swap(value1, value2);
		}
}

template<typename Structure1, typename Structure2, typename Structure3>
void matrix_add(noarr::bag<Structure1>& matrix1, noarr::bag<Structure2>& matrix2, noarr::bag<Structure3>& matrix3)
{
	int x_size = matrix1.layout().template get_length<'x'>();
	int y_size = matrix1.layout().template get_length<'y'>();

	REQUIRE(x_size == matrix2.layout().template get_length<'x'>());
	REQUIRE(y_size == matrix2.layout().template get_length<'y'>());
	REQUIRE(x_size == matrix3.layout().template get_length<'x'>());
	REQUIRE(y_size == matrix3.layout().template get_length<'y'>());

	for (int i = 0; i < x_size; i++)
		for (int j = 0; j < y_size; j++)
		{
			int& value1 = matrix1.layout().template get_at<'x', 'y'>(matrix1.data(), i, j);
			int& value2 = matrix2.layout().template get_at<'x', 'y'>(matrix2.data(), i, j);
			int& value3 = matrix3.layout().template get_at<'x', 'y'>(matrix3.data(), i, j);
			value3 = value1 + value2;
		}
}

template<typename Structure>
void matrix_scalar_multiplication(noarr::bag<Structure>& matrix1, int scalar)
{
	int x_size = matrix1.layout().template get_length<'x'>();
	int y_size = matrix1.layout().template get_length<'y'>();

	for (int i = 0; i < x_size; i++)
		for (int j = i; j < y_size; j++)
			matrix1.layout().template get_at<'x', 'y'>(matrix1.data(), i, j) *= scalar;
}

template<typename Structure1, typename Structure2, typename Structure3>
void matrix_multiply(noarr::bag<Structure1>& matrix1, noarr::bag<Structure2>& matrix2, noarr::bag<Structure3>& matrix3)
{
	int x1_size = matrix1.layout().template get_length<'x'>();
	int y1_size = matrix1.layout().template get_length<'y'>();
	int x2_size = matrix2.layout().template get_length<'x'>();
	int y2_size = matrix2.layout().template get_length<'y'>();
	int x3_size = matrix3.layout().template get_length<'x'>();
	int y3_size = matrix3.layout().template get_length<'y'>();

	REQUIRE(x1_size == y2_size);
	REQUIRE(y1_size == y3_size);
	REQUIRE(x2_size == x3_size);

	for (int i = 0; i < x3_size; i++)
	{
		for (int j = 0; j < y3_size; j++)
		{
			int sum = 0;

			for (int k = 0; k < x1_size; k++)
			{
				int& value1 = matrix1.layout().template get_at<'x', 'y'>(matrix1.data(), k, j);
				int& value2 = matrix2.layout().template get_at<'x', 'y'>(matrix2.data(), i, k);
				sum += value1 * value2;
			}

			matrix3.layout().template get_at<'x', 'y'>(matrix3.data(), i, j) = sum;
		}
	}
}

template<MatrixDataLayout layout>
void matrix_template_test(int size)
{
	auto m1 = noarr::bag(noarr::wrap(GetMatrixStructreStructure<layout>::GetMatrixStructure()).template set_length<'x'>(size).template set_length<'y'>(size));
	auto m2 = noarr::bag(noarr::wrap(GetMatrixStructreStructure<layout>::GetMatrixStructure()).template set_length<'x'>(size).template set_length<'y'>(size));
	auto m3 = noarr::bag(noarr::wrap(GetMatrixStructreStructure<layout>::GetMatrixStructure()).template set_length<'x'>(size).template set_length<'y'>(size));

	matrix_multiply(m1, m2, m3);
}

void matrix_template_test_runtime(MatrixDataLayout layout, int size)
{
	if (layout == MatrixDataLayout::Rows)
		matrix_template_test<MatrixDataLayout::Rows>(size);
	else if (layout == MatrixDataLayout::Columns)
		matrix_template_test<MatrixDataLayout::Columns>(size);
	else if (layout == MatrixDataLayout::Zcurve)
		matrix_template_test<MatrixDataLayout::Zcurve>(size);
}


struct matrix
{
	matrix(int X, int Y, int* Ary) : x(X), y(Y), ary(Ary) {};

	int x;
	int y;
	int* ary;
};

// srand(time(NULL));
matrix* get_clasic_matrix(int x, int y)
{
	const int length = x * y;
	int* ary = new int[length];
	for (int i = 0; i < length; i++)
		ary[i] = rand() % 10;

	return new matrix(x, y, ary);
}

bool are_equal_matrices(matrix& m1, matrix& m2)
{
	if (m1.x != m2.x)
		return false;
	if (m1.y != m2.y)
		return false;

	const int length = m1.x * m1.y;
	for (int i = 0; i < length; i++)
		if (m1.ary[i] != m2.ary[i])
			return false;

	return true;
}

template<typename Structure>
matrix* noarr_matrix_to_clasic(noarr::bag<Structure>& matrix1)
{
	int x_size = matrix1.layout().template get_length<'x'>();
	int y_size = matrix1.layout().template get_length<'y'>();

	matrix* m = get_clasic_matrix(x_size, y_size);

	for (int i = 0; i < x_size; i++)
		for (int j = 0; j < y_size; j++)
			m.arr[i + j * x_size] = matrix1.layout().template get_at<'x', 'y'>(matrix1.data(), i, j);

	return m;
}

template<typename Structure>
int* clasic_matrix_to_naorr(matrix& m1, noarr::bag<Structure>& matrix1)
{
	int x_size = matrix1.layout().template get_length<'x'>();
	int y_size = matrix1.layout().template get_length<'y'>();

	for (int i = 0; i < x_size; i++)
		for (int j = 0; j < y_size; j++)
			matrix1.layout().template get_at<'x', 'y'>(matrix1.data(), i, j) = m.ary[i + j * x_size];

	return m;
}

void clasic_matrix_multiply(matrix& m1, matrix& m2, matrix& m3)
{
	int x1_size = m1.x;
	int y1_size = m1.y;
	int x2_size = m2.x;
	int y2_size = m2.y;
	int x3_size = m3.x;
	int y3_size = m3.y;

	REQUIRE(x1_size == y2_size);
	REQUIRE(y1_size == y3_size);
	REQUIRE(x2_size == x3_size);

	for (int i = 0; i < x3_size; i++)
	{
		for (int j = 0; j < y3_size; j++)
		{
			int sum = 0;

			for (int k = 0; k < x1_size; k++)
			{
				int& value1 = m1.ary[k + j * m1.x]; //matrix1.layout().template get_at<'x', 'y'>(matrix1.data(), k, j);
				int& value2 = m2.ary[i + k * m2.x]; // matrix2.layout().template get_at<'x', 'y'>(matrix2.data(), i, k);
				sum += value1 * value2;
			}

			m3.ary[i + j * m3.x] = sum;// matrix3.layout().template get_at<'x', 'y'>(matrix3.data(), i, j) = sum;
		}
	}
}

template<MatrixDataLayout layout>
void matrix_demo_template(int size)
{

	matrix* m1 = get_clasic_matrix(size, size);
	matrix* m2 = get_clasic_matrix(size, size);
	matrix* m3 = get_clasic_matrix(size, size);

	auto n1 = noarr::bag(noarr::wrap(GetMatrixStructreStructure<layout>::GetMatrixStructure()).template set_length<'x'>(size).template set_length<'y'>(size));
	auto n2 = noarr::bag(noarr::wrap(GetMatrixStructreStructure<layout>::GetMatrixStructure()).template set_length<'x'>(size).template set_length<'y'>(size));
	auto n3 = noarr::bag(noarr::wrap(GetMatrixStructreStructure<layout>::GetMatrixStructure()).template set_length<'x'>(size).template set_length<'y'>(size));

	clasic_matrix_to_naorr(m1, n1);
	clasic_matrix_to_naorr(m2, n2);

	clasic_matrix_multiply(m1, m2, m3);
	matrix_multiply(n1, n2, n3);

	matrix* m4 noarr_matrix_to_clasic(n3);

	REQUIRE(are_equal_matrices(m3, m4));
}

void matrix_demo(MatrixDataLayout layout, int size)
{
	if (layout == MatrixDataLayout::Rows)
		matrix_demo_template<MatrixDataLayout::Rows>(size);
	else if (layout == MatrixDataLayout::Columns)
		matrix_demo_template<MatrixDataLayout::Columns>(size);
	else if (layout == MatrixDataLayout::Zcurve)
		matrix_demo_template<MatrixDataLayout::Zcurve>(size);
}

TEST_CASE("Small matrix demo", "[Small matrix demo]")
{
	matrix_demo(MatrixDataLayout::Rows, 10);
}

TEST_CASE("Small matrix multimplication Rows", "[Small matrix multimplication Rows]")
{
	matrix_template_test_runtime(MatrixDataLayout::Rows, 10);
}

TEST_CASE("Small matrix multimplication Columns", "[Small matrix multimplication Columns]")
{
	matrix_template_test_runtime(MatrixDataLayout::Columns, 10);
}

TEST_CASE("Small matrix multimplication Zcurve", "[Small matrix multimplication Zcurve]")
{
	matrix_template_test_runtime(MatrixDataLayout::Zcurve, 10);
}






// s��t�n�, n�sobit matice, n�sobit scal�rem, 

/*struct MatrixStruct {
	virtual ~MatrixStruct() noexcept = default;
	virtual MatrixDataLayout GetLayout() = 0;
};
template<typename layout>
struct MatrixDerived;
struct RowsStruct {};
struct ColumnsStruct {};
struct ZcurveStruct {};

template<>
struct MatrixDerived<RowsStruct> : MatrixStruct {
	MatrixDataLayout GetLayout() override { return MatrixDataLayout::Rows; }
};

template<>
struct MatrixDerived<ColumnsStruct> : MatrixStruct {
	MatrixDataLayout GetLayout() override { return MatrixDataLayout::Columns; }
};

template<>
struct MatrixDerived<ZcurveStruct> : MatrixStruct {
	MatrixDataLayout GetLayout() override { return MatrixDataLayout::Zcurve; }
};

static constexpr auto GetImageStructure(RowsStruct)
{
	return noarr::vector<'x', noarr::vector<'y', noarr::scalar<int>>>();
}

static constexpr auto GetImageStructure(ColumnsStruct)
{
	return noarr::vector<'y', noarr::vector<'x', noarr::scalar<int>>>();
}

static constexpr auto GetMatrixStructure(ZcurveStruct)
{
	return noarr::vector<'x', noarr::vector<'y', noarr::scalar<int>>>();
}*/




/*template<typename Structure>
auto noarr::bag(noarr::wrapper<Structure> s, MatrixDataLayout l)
{
	return noarr::bag<Structure>(s.unwrap(), l);
}

template<typename Structure, typename layout>
auto GetMatrix(int x_size, int y_size)
{
	return noarr::bag(noarr::wrap(GetMatrixStructreStructure<layout>::GetImageStructure(), layout).template set_length<'x'>(x_size).template set_length<'y'>(y_size));
}*/





/*template<typename Structure>
auto matrix_transpose(noarr::bag<Structure>& matrix)
{
	MatrixDataLayout layout = matrix.dataLayout();

	if (layout == MatrixDataLayout::Rows)
		return matrix_transpose<MatrixDataLayout::Rows>(matrix);
	else if (layout == MatrixDataLayout::Columns)
		return matrix_transpose<MatrixDataLayout::Columns>(matrix);
	else if (layout == MatrixDataLayout::Zcurve)
		return matrix_transpose<MatrixDataLayout::Zcurve>(matrix);
}

template<typename Structure1, typename targetLayout>
auto matrix_clone(noarr::bag<Structure1>& matrix1)
{
	int x_size = matrix1.layout().template get_length<'x'>();
	int y_size = matrix1.layout().template get_length<'y'>();
	auto matrix2 = GetMatrix<targetLayout>(x_size, y_size);
	matrix_copy(matrix1, matrix2);
	return matrix2;
}

template<typename Structure>
auto matrix_clone(noarr::bag<Structure>& matrix, MatrixDataLayout targetLayout)
{
	MatrixDataLayout layout = matrix.dataLayout();

	if (layout == MatrixDataLayout::Rows)
	{
		if (targetLayout == MatrixDataLayout::Rows)
			return matrix_clone<MatrixDataLayout::Rows, MatrixDataLayout::Rows>(matrix);
		else if (targetLayout == MatrixDataLayout::Columns)
			return matrix_clone<MatrixDataLayout::Rows, MatrixDataLayout::Columns>(matrix);
		else if (targetLayout == MatrixDataLayout::Zcurve)
			return matrix_clone<MatrixDataLayout::Rows, MatrixDataLayout::Zcurve>(matrix);
	}
	else if (layout == MatrixDataLayout::Columns)
	{
		if (targetLayout == MatrixDataLayout::Rows)
			return matrix_clone<MatrixDataLayout::Columns, MatrixDataLayout::Rows>(matrix);
		else if (targetLayout == MatrixDataLayout::Columns)
			return matrix_clone<MatrixDataLayout::Columns, MatrixDataLayout::Columns>(matrix);
		else if (targetLayout == MatrixDataLayout::Zcurve)
			return matrix_clone<MatrixDataLayout::Columns, MatrixDataLayout::Zcurve>(matrix);
	}
	else if (layout == MatrixDataLayout::Zcurve)
	{
		if (targetLayout == MatrixDataLayout::Rows)
			return matrix_clone<MatrixDataLayout::Zcurve, MatrixDataLayout::Rows>(matrix);
		else if (targetLayout == MatrixDataLayout::Columns)
			return matrix_clone<MatrixDataLayout::Zcurve, MatrixDataLayout::Columns>(matrix);
		else if (targetLayout == MatrixDataLayout::Zcurve)
			return matrix_clone<MatrixDataLayout::Zcurve, MatrixDataLayout::Zcurve>(matrix);
	}
}*/


/*template<MatrixDataLayout layout, std::size_t width, std::size_t height, std::size_t pixel_range = 256>
void histogram_template_test()
{
	auto image = noarr::bag(noarr::wrap(GetMatrixStructreStructure<layout>::GetImageStructure()).template set_length<'x'>(width).template set_length<'y'>(height));
	CHECK(image.layout().get_size() == width * height * sizeof(int));

	int y_size = image.layout().template get_length<'y'>();
	CHECK(y_size == height);

	auto histogram = noarr::bag(noarr::array<'x', pixel_range, noarr::scalar<int>>());
	CHECK(histogram.layout().get_size() == pixel_range * sizeof(int));

	image.clear();
	histogram.clear();

	int x_size = image.layout().template get_length<'x'>();
	REQUIRE(x_size == width);

	y_size = image.layout().template get_length<'y'>();
	REQUIRE(y_size == height);

	for (int i = 0; i < x_size; i++)
	{
		for (int j = 0; j < y_size; j++)
		{
			//int& pixel_value = *((int*)(image.blob + x_fixed.fix<'y'>(j).offset())); // v1
			//int& pixel_value = *((int*)x_fixed.fix<'y'>(j).get_at(image.blob)); // v2
			int pixel_value = image.layout().template get_at<'x','y'>(image.data(), i, j); // v3

			if (pixel_value != 0)
				FAIL();

			int& histogram_value = histogram.layout().template get_at<'x'>(histogram.data(), pixel_value);
			histogram_value = histogram_value + 1;
		}
	}
}

TEST_CASE("Matrix prototype 720 x 480 with 16 colors", "[Matrix prototype]")
{
	histogram_template_test<MatrixDataLayout::Rows, 720, 480, 16>();
}

TEST_CASE("Matrix prototype 720 x 480", "[Matrix prototype]")
{
	histogram_template_test<MatrixDataLayout::Rows, 720, 480>();
}

TEST_CASE("Matrix prototype 1080 x 720", "[Matrix prototype]")
{
	histogram_template_test<MatrixDataLayout::Rows, 1080, 720>();
}

TEST_CASE("Matrix prototype 1920 x 1080", "[Matrix prototype]")
{
	histogram_template_test<MatrixDataLayout::Rows, 1920, 1080>();
}

template<MatrixDataLayout layout, std::size_t width, std::size_t height, std::size_t pixel_range = 256>
void histogram_template_test_clear()
{
	auto image = noarr::bag(noarr::wrap(GetMatrixStructreStructure<layout>::GetImageStructure()).template set_length<'x'>(width).template set_length<'y'>(height)); // image size 
	auto histogram = noarr::bag(noarr::array<'x', pixel_range, noarr::scalar<int>>()); // lets say that every image has 256 pixel_range

	image.clear();
	histogram.clear();

	int x_size = image.layout().template get_length<'x'>();
	int y_size = image.layout().template get_length<'y'>();

	for (int i = 0; i < x_size; i++)
	{
		for (int j = 0; j < y_size; j++)
		{
			int pixel_value = image.layout().template get_at<'x', 'y'>(image.data(), i, j);

			int& histogram_value = histogram.layout().template get_at<'x'>(histogram.data(), pixel_value);
			histogram_value = histogram_value + 1;
		}
	}
}*/






/*struct MatrixStruct {
	virtual ~MatrixStruct() noexcept = default;
	virtual MatrixDataLayout GetLayout() = 0;
};
template<typename layout>
struct MatrixDerived;
struct RowsStruct {};
struct ColumnsStruct {};
struct ZcurveStruct {};

template<>
struct MatrixDerived<RowsStruct> : MatrixStruct {
	MatrixDataLayout GetLayout() override { return MatrixDataLayout::Rows; }
};

template<>
struct MatrixDerived<ColumnsStruct> : MatrixStruct {
	MatrixDataLayout GetLayout() override { return MatrixDataLayout::Columns; }
};

template<>
struct MatrixDerived<ZcurveStruct> : MatrixStruct {
	MatrixDataLayout GetLayout() override { return MatrixDataLayout::Zcurve; }
};

static constexpr auto GetImageStructure(RowsStruct)
{
	return noarr::vector<'x', noarr::vector<'y', noarr::scalar<int>>>();
}

static constexpr auto GetImageStructure(ColumnsStruct)
{
	return noarr::vector<'y', noarr::vector<'x', noarr::scalar<int>>>();
}

static constexpr auto GetMatrixStructure(ZcurveStruct)
{
	return noarr::vector<'x', noarr::vector<'y', noarr::scalar<int>>>();
}*/



