
/*
  +------------------------------------------------------------------------+
  | Zephir Language                                                        |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2015 Zephir Team (http://www.zephir-lang.com)       |
  +------------------------------------------------------------------------+
  | This source file is subject to the New BSD License that is bundled     |
  | with this package in the file docs/LICENSE.txt.                        |
  |                                                                        |
  | If you did not receive a copy of the license and are unable to         |
  | obtain it through the world-wide-web, please send an email             |
  | to license@zephir-lang.com so we can send you a copy immediately.      |
  +------------------------------------------------------------------------+
  | Authors: Andres Gutierrez <andres@zephir-lang.com>                     |
  |          Eduar Carvajal <eduar@zephir-lang.com>                        |
  |          Vladimir Kolesnikov <vladimir@extrememember.com>              |
  +------------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "ext/standard/php_string.h"
#include "ext/standard/php_math.h"
#include "php_ext.h"
#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/string.h"
#include "kernel/operators.h"

#include "Zend/zend_operators.h"

void zephir_make_printable_zval(zval *expr, zval *expr_copy, int *use_copy){
	zend_make_printable_zval(expr, expr_copy, use_copy);
	if (use_copy) {
		Z_SET_REFCOUNT_P(expr_copy, 1);
		Z_UNSET_ISREF_P(expr_copy);
	}
}

/**
 * Performs logical AND function operator
 */
int zephir_and_function(zval *result, zval *left, zval *right){
	int istrue = zend_is_true(left) && zend_is_true(right);
	ZVAL_BOOL(result, istrue);
	return SUCCESS;
}

/**
 * Appends the content of the right operator to the left operator
 */
void zephir_concat_self(zval **left, zval *right TSRMLS_DC){

	zval left_copy, right_copy;
	uint length;
	int use_copy_left = 0, use_copy_right = 0;

	if (Z_TYPE_P(right) != IS_STRING) {
		zephir_make_printable_zval(right, &right_copy, &use_copy_right);
		if (use_copy_right) {
			right = &right_copy;
		}
	}

	if (Z_TYPE_PP(left) == IS_NULL) {

		Z_STRVAL_PP(left) = emalloc(Z_STRLEN_P(right) + 1);
		memcpy(Z_STRVAL_PP(left), Z_STRVAL_P(right), Z_STRLEN_P(right));
		Z_STRVAL_PP(left)[Z_STRLEN_P(right)] = 0;
		Z_STRLEN_PP(left) = Z_STRLEN_P(right);
		Z_TYPE_PP(left) = IS_STRING;

		if (use_copy_right) {
			zval_dtor(&right_copy);
		}

		return;
	}

	if (Z_TYPE_PP(left) != IS_STRING) {
		zephir_make_printable_zval(*left, &left_copy, &use_copy_left);
		if (use_copy_left) {
			ZEPHIR_CPY_WRT_CTOR(*left, (&left_copy));
		}
	}

	SEPARATE_ZVAL_IF_NOT_REF(left);

	length = Z_STRLEN_PP(left) + Z_STRLEN_P(right);
	Z_STRVAL_PP(left) = str_erealloc(Z_STRVAL_PP(left), length + 1);

	memcpy(Z_STRVAL_PP(left) + Z_STRLEN_PP(left), Z_STRVAL_P(right), Z_STRLEN_P(right));
	Z_STRVAL_PP(left)[length] = 0;
	Z_STRLEN_PP(left) = length;
	Z_TYPE_PP(left) = IS_STRING;

	if (use_copy_left) {
		zval_dtor(&left_copy);
	}

	if (use_copy_right) {
		zval_dtor(&right_copy);
	}
}

/**
 * Appends the content of the right operator to the left operator
 */
void zephir_concat_self_str(zval **left, const char *right, int right_length TSRMLS_DC){

	zval left_copy;
	uint length;
	int use_copy = 0;

	if (Z_TYPE_PP(left) == IS_NULL) {

		Z_STRVAL_PP(left) = emalloc(right_length + 1);
		memcpy(Z_STRVAL_PP(left), right, right_length);
		Z_STRVAL_PP(left)[right_length] = 0;
		Z_STRLEN_PP(left) = right_length;
		Z_TYPE_PP(left) = IS_STRING;

		return;
	}

	if (Z_TYPE_PP(left) != IS_STRING) {
		zephir_make_printable_zval(*left, &left_copy, &use_copy);
		if (use_copy) {
			ZEPHIR_CPY_WRT_CTOR(*left, (&left_copy));
		}
	}

	SEPARATE_ZVAL_IF_NOT_REF(left);

	length = Z_STRLEN_PP(left) + right_length;
	Z_STRVAL_PP(left) = str_erealloc(Z_STRVAL_PP(left), length + 1);

	memcpy(Z_STRVAL_PP(left) + Z_STRLEN_PP(left), right, right_length);
	Z_STRVAL_PP(left)[length] = 0;
	Z_STRLEN_PP(left) = length;
	Z_TYPE_PP(left) = IS_STRING;

	if (use_copy) {
		zval_dtor(&left_copy);
	}
}

/**
 * Appends the content of the right operator to the left operator
 */
void zephir_concat_self_long(zval **left, const long right TSRMLS_DC) {

	zval left_copy;
	uint length;
	char *right_char;
	int use_copy = 0, right_length = 0;

	right_length = zephir_spprintf(&right_char, 0, "%ld", right);

	if (Z_TYPE_PP(left) == IS_NULL) {
		Z_STRVAL_PP(left) = emalloc(right_length + 1);
		if (right_length > 0) {
			memcpy(Z_STRVAL_PP(left), right_char, right_length);
		} else {
			memcpy(Z_STRVAL_PP(left), "", 0);
		}
		Z_STRVAL_PP(left)[right_length] = 0;
		Z_STRLEN_PP(left) = right_length;
		Z_TYPE_PP(left) = IS_STRING;
		return;
	}

	if (Z_TYPE_PP(left) != IS_STRING) {
		zephir_make_printable_zval(*left, &left_copy, &use_copy);
		if (use_copy) {
			ZEPHIR_CPY_WRT_CTOR(*left, (&left_copy));
		}
	}

	if (right_length > 0) {

		SEPARATE_ZVAL_IF_NOT_REF(left);

		length = Z_STRLEN_PP(left) + right_length;
		Z_STRVAL_PP(left) = str_erealloc(Z_STRVAL_PP(left), length + 1);
		memcpy(Z_STRVAL_PP(left) + Z_STRLEN_PP(left), right_char, right_length);
		Z_STRVAL_PP(left)[length] = 0;
		Z_STRLEN_PP(left) = length;
		Z_TYPE_PP(left) = IS_STRING;
	}

	if (use_copy) {
		zval_dtor(&left_copy);
	}
}

/**
 * Appends the content of the right operator to the left operator
 */
void zephir_concat_self_char(zval **left, unsigned char right TSRMLS_DC) {

	zval left_copy;
	int use_copy = 0;

	if (Z_TYPE_PP(left) == IS_NULL) {
		Z_STRVAL_PP(left) = emalloc(2);
		Z_STRVAL_PP(left)[0] = right;
		Z_STRVAL_PP(left)[1] = 0;
		Z_STRLEN_PP(left) = 1;
		Z_TYPE_PP(left) = IS_STRING;
		return;
	}

	if (Z_TYPE_PP(left) != IS_STRING) {
		zephir_make_printable_zval(*left, &left_copy, &use_copy);
		if (use_copy) {
			ZEPHIR_CPY_WRT_CTOR(*left, (&left_copy));
		}
	}

	SEPARATE_ZVAL_IF_NOT_REF(left);

	Z_STRLEN_PP(left)++;
	Z_STRVAL_PP(left) = str_erealloc(Z_STRVAL_PP(left), Z_STRLEN_PP(left) + 1);
	Z_STRVAL_PP(left)[Z_STRLEN_PP(left) - 1] = right;
	Z_STRVAL_PP(left)[Z_STRLEN_PP(left)] = 0;
	Z_TYPE_PP(left) = IS_STRING;

	if (use_copy) {
		zval_dtor(&left_copy);
	}
}

/**
 * Natural compare with string operandus on right
 */
int zephir_compare_strict_string(zval *op1, const char *op2, int op2_length) {

	switch (Z_TYPE_P(op1)) {

		case IS_STRING:
			if (!Z_STRLEN_P(op1) && !op2_length) {
				return 1;
			}
			if (Z_STRLEN_P(op1) != op2_length) {
				return 0;
			}
			return !zend_binary_strcmp(Z_STRVAL_P(op1), Z_STRLEN_P(op1), op2, op2_length);

		case IS_NULL:
			return !zend_binary_strcmp("", 0, op2, op2_length);

		case IS_BOOL:
			if (!Z_BVAL_P(op1)) {
				return !zend_binary_strcmp("0", strlen("0"), op2, op2_length);
			} else {
				return !zend_binary_strcmp("1", strlen("1"), op2, op2_length);
			}
	}

	return 0;
}

/**
 * Natural compare with long operandus on right
 */
int zephir_compare_strict_long(zval *op1, long op2 TSRMLS_DC) {

	int bool_result;

	switch (Z_TYPE_P(op1)) {
		case IS_LONG:
			return Z_LVAL_P(op1) == op2;
		case IS_DOUBLE:
			return Z_DVAL_P(op1) == (double) op2;
		case IS_NULL:
			return 0 == op2;
		case IS_BOOL:
			if (Z_BVAL_P(op1)) {
				return 1 == op2;
			} else {
				return 0 == op2;
			}
		default:
			{
				zval result, op2_tmp;
				ZVAL_LONG(&op2_tmp, op2);
				is_equal_function(&result, op1, &op2_tmp TSRMLS_CC);
				bool_result = Z_BVAL(result);
				return bool_result;
			}
	}

	return 0;
}

/**
 * Natural compare with double operandus on right
 */
int zephir_compare_strict_double(zval *op1, double op2 TSRMLS_DC) {

	int bool_result;

	switch (Z_TYPE_P(op1)) {
		case IS_LONG:
			return Z_LVAL_P(op1) == (long) op2;
		case IS_DOUBLE:
			return Z_DVAL_P(op1) == op2;
		case IS_NULL:
			return 0 == op2;
		case IS_BOOL:
			if (Z_BVAL_P(op1)) {
				return 1 == op2;
			} else {
				return 0 == op2;
			}
		default:
			{
				zval result, op2_tmp;
				ZVAL_DOUBLE(&op2_tmp, op2);
				is_equal_function(&result, op1, &op2_tmp TSRMLS_CC);
				bool_result = Z_BVAL(result);
				return bool_result;
			}
	}

	return 0;
}

/**
 * Natural compare with bool operandus on right
 */
int zephir_compare_strict_bool(zval *op1, zend_bool op2 TSRMLS_DC) {

	int bool_result;

	switch (Z_TYPE_P(op1)) {
		case IS_LONG:
			return (Z_LVAL_P(op1) ? 1 : 0) == op2;
		case IS_DOUBLE:
			return (Z_DVAL_P(op1) ? 1 : 0) == op2;
		case IS_NULL:
			return 0 == op2;
		case IS_BOOL:
			if (Z_BVAL_P(op1)) {
				return 1 == op2;
			} else {
				return 0 == op2;
			}
		default:
			{
				zval result, op2_tmp;
				ZVAL_BOOL(&op2_tmp, op2);
				is_equal_function(&result, op1, &op2_tmp TSRMLS_CC);
				bool_result = Z_BVAL(result);
				return bool_result;
			}
	}

	return 0;
}

/**
 * Do add function keeping ref_count and is_ref
 */
int zephir_add_function_ex(zval *result, zval *op1, zval *op2 TSRMLS_DC) {
	int status;
	int ref_count = Z_REFCOUNT_P(result);
	int is_ref = Z_ISREF_P(result);
	status = add_function(result, op1, op2 TSRMLS_CC);
	Z_SET_REFCOUNT_P(result, ref_count);
	Z_SET_ISREF_TO_P(result, is_ref);
	return status;
}

void zephir_negate(zval *z TSRMLS_DC) {
	while (1) {
		switch (Z_TYPE_P(z)) {
			case IS_LONG:
			case IS_BOOL:
				ZVAL_LONG(z, -Z_LVAL_P(z));
				return;

			case IS_DOUBLE:
				ZVAL_DOUBLE(z, -Z_DVAL_P(z));
				return;

			case IS_NULL:
				ZVAL_LONG(z, 0);
				return;

			default:
				convert_scalar_to_number(z TSRMLS_CC);
				assert(Z_TYPE_P(z) == IS_LONG || Z_TYPE_P(z) == IS_DOUBLE);
		}
	}
}

void zephir_convert_to_object(zval *op) {
    convert_to_object(op);
}

/**
 * Cast variables converting they to other types
 */
void zephir_cast(zval *result, zval *var, zend_uint type){

	ZVAL_ZVAL(result, var, 1, 0);

	switch (type) {
		case IS_STRING:
			convert_to_string(result);
			break;
		case IS_LONG:
			convert_to_long(result);
			break;
		case IS_DOUBLE:
			convert_to_double(result);
			break;
		/*case IS_BOOL:
			convert_to_bool(result);
			break;*/
		case IS_ARRAY:
			if (Z_TYPE_P(result) != IS_ARRAY) {
				convert_to_array(result);
			}
			break;
	}

}

/**
 * Returns the long value of a zval
 */
long zephir_get_intval_ex(const zval *op) {

	switch (Z_TYPE_P(op)) {
        case IS_ARRAY:
            return zend_hash_num_elements(Z_ARRVAL_P(op)) ? 1 : 0;
            break;

#if PHP_VERSION_ID > 50400
	    case IS_CALLABLE:
#endif
	    case IS_RESOURCE:
	    case IS_OBJECT:
	        return 1;

		case IS_LONG:
			return Z_LVAL_P(op);

		case IS_BOOL:
			return Z_BVAL_P(op);

		case IS_DOUBLE:
			return (long) Z_DVAL_P(op);

		case IS_STRING: {
			long long_value = 0;
			double double_value = 0;
			zend_uchar type;

			ASSUME(Z_STRVAL_P(op) != NULL);
			type = is_numeric_string(Z_STRVAL_P(op), Z_STRLEN_P(op), &long_value, &double_value, 0);
			if (type == IS_LONG) {
				return long_value;
			}
			if (type == IS_DOUBLE) {
				return (long) double_value;
			}
			return 0;
		}
	}

	return 0;
}

/**
 * Returns the long value of a zval
 */
double zephir_get_doubleval_ex(const zval *op) {

	int type;
	long long_value = 0;
	double double_value = 0;

	switch (Z_TYPE_P(op)) {
        case IS_ARRAY:
            return zend_hash_num_elements(Z_ARRVAL_P(op)) ? (double) 1 : 0;
            break;
#if PHP_VERSION_ID > 50400
	    case IS_CALLABLE:
#endif
	    case IS_RESOURCE:
	    case IS_OBJECT:
	        return (double) 1;
		case IS_LONG:
			return (double) Z_LVAL_P(op);
		case IS_BOOL:
			return (double) Z_BVAL_P(op);
		case IS_DOUBLE:
			return Z_DVAL_P(op);
		case IS_STRING:
			if ((type = is_numeric_string(Z_STRVAL_P(op), Z_STRLEN_P(op), &long_value, &double_value, 0))) {
				if (type == IS_LONG) {
					return (double) long_value;
				} else {
					if (type == IS_DOUBLE) {
						return double_value;
					} else {
						return 0;
					}
				}
			}
	}

	return 0;
}

/**
 * Returns the long value of a zval
 */
zend_bool zephir_get_boolval_ex(const zval *op) {

	int type;
	long long_value = 0;
	double double_value = 0;

	switch (Z_TYPE_P(op)) {
        case IS_ARRAY:
            return zend_hash_num_elements(Z_ARRVAL_P(op)) ? (zend_bool) 1 : 0;
            break;
#if PHP_VERSION_ID > 50400
	    case IS_CALLABLE:
#endif
	    case IS_RESOURCE:
	    case IS_OBJECT:
	        return (zend_bool) 1;
		case IS_LONG:
			return (Z_LVAL_P(op) ? (zend_bool) 1 : 0);
		case IS_BOOL:
			return Z_BVAL_P(op);
		case IS_DOUBLE:
			return (Z_DVAL_P(op) ? (zend_bool) 1 : 0);
		case IS_STRING:
			if ((type = is_numeric_string(Z_STRVAL_P(op), Z_STRLEN_P(op), &long_value, &double_value, 0))) {
				if (type == IS_LONG) {
					return (long_value ? (zend_bool) 1 : 0);
				} else {
					if (type == IS_DOUBLE) {
						return (double_value ? (zend_bool) 1 : 0);
					} else {
						return 0;
					}
				}
			}
	}

	return 0;
}

/**
 * Returns the long value of a zval
 */
int zephir_is_numeric_ex(const zval *op) {

	int type;

	switch (Z_TYPE_P(op)) {
		case IS_LONG:
			return 1;
		case IS_BOOL:
			return 0;
		case IS_DOUBLE:
			return 1;
		case IS_STRING:
			if ((type = is_numeric_string(Z_STRVAL_P(op), Z_STRLEN_P(op), NULL, NULL, 0))) {
				if (type == IS_LONG || type == IS_DOUBLE) {
					return 1;
				}
			}
	}

	return 0;
}

/**
 * Check if two zvals are equal
 */
int zephir_is_equal(zval *op1, zval *op2 TSRMLS_DC) {
	zval result;
	#if PHP_VERSION_ID < 50400
	is_equal_function(&result, op1, op2 TSRMLS_CC);
	return Z_BVAL(result);
	#else
	return fast_equal_function(&result, op1, op2 TSRMLS_CC);
	#endif
}

/**
 * Check if a zval is less than other
 */
int zephir_less(zval *op1, zval *op2 TSRMLS_DC) {
	zval result;
	#if PHP_VERSION_ID < 50400
	is_smaller_function(&result, op1, op2 TSRMLS_CC);
	return Z_BVAL(result);
	#else
	return fast_is_smaller_function(&result, op1, op2 TSRMLS_CC);
	#endif
}

/**
 * Check if a zval is less/equal than other
 */
int zephir_less_equal(zval *op1, zval *op2 TSRMLS_DC) {
	zval result;
	is_smaller_or_equal_function(&result, op1, op2 TSRMLS_CC);
	return Z_BVAL(result);
}

/**
 * Check if a zval is less than a long value
 */
int zephir_less_long(zval *op1, long op2 TSRMLS_DC) {
	zval result, op2_zval;
	ZVAL_LONG(&op2_zval, op2);

	is_smaller_function(&result, op1, &op2_zval TSRMLS_CC);
	return Z_BVAL(result);
}

/**
 * Check if a zval is less than a double value
 */
int zephir_less_double(zval *op1, double op2 TSRMLS_DC) {
	zval result, op2_zval;
	ZVAL_DOUBLE(&op2_zval, op2);

	is_smaller_function(&result, op1, &op2_zval TSRMLS_CC);
	return Z_BVAL(result);
}

int zephir_less_equal_long(zval *op1, long op2 TSRMLS_DC) {
	zval result, op2_zval;
	ZVAL_LONG(&op2_zval, op2);

	is_smaller_or_equal_function(&result, op1, &op2_zval TSRMLS_CC);
	return Z_BVAL(result);
}

/**
 * Check if a zval is greater than other
 */
int zephir_greater(zval *op1, zval *op2 TSRMLS_DC) {
	zval result;
	is_smaller_or_equal_function(&result, op1, op2 TSRMLS_CC);
	return !Z_BVAL(result);
}

/**
 * Check if a zval is greater than a long value
 */
int zephir_greater_long(zval *op1, long op2 TSRMLS_DC) {
	zval result, op2_zval;
	ZVAL_LONG(&op2_zval, op2);

	is_smaller_or_equal_function(&result, op1, &op2_zval TSRMLS_CC);
	return !Z_BVAL(result);
}

/**
 * Check if a zval is greater than a double value
 */
int zephir_greater_double(zval *op1, double op2 TSRMLS_DC) {
	zval result, op2_zval;
	ZVAL_DOUBLE(&op2_zval, op2);

	is_smaller_or_equal_function(&result, op1, &op2_zval TSRMLS_CC);
	return !Z_BVAL(result);
}

/**
 * Check if a zval is greater/equal than other
 */
int zephir_greater_equal(zval *op1, zval *op2 TSRMLS_DC) {
	zval result;
	is_smaller_function(&result, op1, op2 TSRMLS_CC);
	return !Z_BVAL(result);
}

/**
 * Check for greater/equal
 */
int zephir_greater_equal_long(zval *op1, long op2 TSRMLS_DC) {
	zval result, op2_zval;
	ZVAL_LONG(&op2_zval, op2);
	is_smaller_function(&result, op1, &op2_zval TSRMLS_CC);
	return !Z_BVAL(result);
}

/**
 * Check if two zvals are identical
 */
int zephir_is_identical(zval *op1, zval *op2 TSRMLS_DC) {
	zval result;
	is_identical_function(&result, op1, op2 TSRMLS_CC);
	return Z_BVAL(result);
}

/**
 * Do bitwise_and function keeping ref_count and is_ref
 */
int zephir_bitwise_and_function(zval *result, zval *op1, zval *op2 TSRMLS_DC){
	int status;
	int ref_count = Z_REFCOUNT_P(result);
	int is_ref = Z_ISREF_P(result);
	status = bitwise_and_function(result, op1, op2 TSRMLS_CC);
	Z_SET_REFCOUNT_P(result, ref_count);
	Z_SET_ISREF_TO_P(result, is_ref);
	return status;
}

/**
 * Do bitwise_or function keeping ref_count and is_ref
 */
int zephir_bitwise_or_function(zval *result, zval *op1, zval *op2 TSRMLS_DC){
	int status;
	int ref_count = Z_REFCOUNT_P(result);
	int is_ref = Z_ISREF_P(result);
	status = bitwise_or_function(result, op1, op2 TSRMLS_CC);
	Z_SET_REFCOUNT_P(result, ref_count);
	Z_SET_ISREF_TO_P(result, is_ref);
	return status;
}

/**
 * Do bitwise_xor function keeping ref_count and is_ref
 */
int zephir_bitwise_xor_function(zval *result, zval *op1, zval *op2 TSRMLS_DC){
	int status;
	int ref_count = Z_REFCOUNT_P(result);
	int is_ref = Z_ISREF_P(result);
	status = bitwise_xor_function(result, op1, op2 TSRMLS_CC);
	Z_SET_REFCOUNT_P(result, ref_count);
	Z_SET_ISREF_TO_P(result, is_ref);
	return status;
}

/**
 * Do shiftleft function keeping ref_count and is_ref
 */
int zephir_shift_left_function(zval *result, zval *op1, zval *op2 TSRMLS_DC){
	int status;
	int ref_count = Z_REFCOUNT_P(result);
	int is_ref = Z_ISREF_P(result);
	status = shift_left_function(result, op1, op2 TSRMLS_CC);
	Z_SET_REFCOUNT_P(result, ref_count);
	Z_SET_ISREF_TO_P(result, is_ref);
	return status;
}

/**
 * Do shiftright function keeping ref_count and is_ref
 */
int zephir_shift_right_function(zval *result, zval *op1, zval *op2 TSRMLS_DC){
	int status;
	int ref_count = Z_REFCOUNT_P(result);
	int is_ref = Z_ISREF_P(result);
	status = shift_right_function(result, op1, op2 TSRMLS_CC);
	Z_SET_REFCOUNT_P(result, ref_count);
	Z_SET_ISREF_TO_P(result, is_ref);
	return status;
}

/**
 * Do safe divisions between two longs
 */
double zephir_safe_div_long_long(long op1, long op2 TSRMLS_DC) {
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	return (double) op1 / (double) op2;
}

/**
 * Do safe divisions between two long/double
 */
double zephir_safe_div_long_double(long op1, double op2 TSRMLS_DC) {
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	return (double) op1 / op2;
}

/**
 * Do safe divisions between two double/long
 */
double zephir_safe_div_double_long(double op1, long op2 TSRMLS_DC) {
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	return op1 / (double) op2;
}

/**
 * Do safe divisions between two doubles
 */
double zephir_safe_div_double_double(double op1, double op2 TSRMLS_DC) {
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	return op1 / op2;
}

/**
 * Do safe divisions between two zval/long
 */
double zephir_safe_div_zval_long(zval *op1, long op2 TSRMLS_DC) {
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	switch (Z_TYPE_P(op1)) {
		case IS_ARRAY:
		case IS_OBJECT:
		case IS_RESOURCE:
			zend_error(E_WARNING, "Unsupported operand types");
			break;
	}
	return ((double) zephir_get_numberval(op1)) / (double) op2;
}

/**
 * Do safe divisions between two zval/double
 */
double zephir_safe_div_zval_double(zval *op1, double op2 TSRMLS_DC) {
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	switch (Z_TYPE_P(op1)) {
		case IS_ARRAY:
		case IS_OBJECT:
		case IS_RESOURCE:
			zend_error(E_WARNING, "Unsupported operand types");
			break;
	}
	return ((double) zephir_get_numberval(op1)) / op2;
}

/**
 * Do safe divisions between two long/zval
 */
double zephir_safe_div_long_zval(long op1, zval *op2 TSRMLS_DC) {
	if (!zephir_get_numberval(op2)) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	switch (Z_TYPE_P(op2)) {
		case IS_ARRAY:
		case IS_OBJECT:
		case IS_RESOURCE:
			zend_error(E_WARNING, "Unsupported operand types");
			break;
	}
	return (double) op1 / ((double) zephir_get_numberval(op2));
}

/**
 * Do safe divisions between two double/zval
 */
double zephir_safe_div_double_zval(double op1, zval *op2 TSRMLS_DC) {
	if (!zephir_get_numberval(op2)) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	switch (Z_TYPE_P(op2)) {
		case IS_ARRAY:
		case IS_OBJECT:
		case IS_RESOURCE:
			zend_error(E_WARNING, "Unsupported operand types");
			break;
	}
	return op1 / ((double) zephir_get_numberval(op2));
}

double zephir_floor(zval *op1 TSRMLS_DC)
{
	convert_scalar_to_number_ex(&op1);

	if (Z_TYPE_PP(&op1) == IS_DOUBLE) {
		return floor(Z_DVAL_PP(&op1));
	} else if (Z_TYPE_PP(&op1) == IS_LONG) {
		convert_to_double_ex(&op1);
		return Z_DVAL_PP(&op1);
	}
	return 0;
}

/**
 * Do safe divisions between two longs
 */
long zephir_safe_mod_long_long(long op1, long op2 TSRMLS_DC) {
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	return op1 % op2;
}

/**
 * Do safe divisions between two long/double
 */
long zephir_safe_mod_long_double(long op1, double op2 TSRMLS_DC) {
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	return op1 % (long) op2;
}

/**
 * Do safe divisions between two double/long
 */
long zephir_safe_mod_double_long(double op1, long op2 TSRMLS_DC) {
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	return (long) op1 % op2;
}

/**
 * Do safe divisions between two doubles
 */
long zephir_safe_mod_double_double(double op1, double op2 TSRMLS_DC) {
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	return (long) op1 % (long) op2;
}

/**
 * Do safe divisions between two zval/long
 */
long zephir_safe_mod_zval_long(zval *op1, long op2 TSRMLS_DC) {
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	switch (Z_TYPE_P(op1)) {
		case IS_ARRAY:
		case IS_OBJECT:
		case IS_RESOURCE:
			zend_error(E_WARNING, "Unsupported operand types");
			break;
	}
	return ((long) zephir_get_numberval(op1)) % (long) op2;
}

/**
 * Do safe divisions between two zval/double
 */
long zephir_safe_mod_zval_double(zval *op1, double op2 TSRMLS_DC) {
	if (!op2) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	switch (Z_TYPE_P(op1)) {
		case IS_ARRAY:
		case IS_OBJECT:
		case IS_RESOURCE:
			zend_error(E_WARNING, "Unsupported operand types");
			break;
	}
	return ((long) zephir_get_numberval(op1)) % (long) op2;
}

/**
 * Do safe divisions between two long/zval
 */
long zephir_safe_mod_long_zval(long op1, zval *op2 TSRMLS_DC) {
	if (!zephir_get_numberval(op2)) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	switch (Z_TYPE_P(op2)) {
		case IS_ARRAY:
		case IS_OBJECT:
		case IS_RESOURCE:
			zend_error(E_WARNING, "Unsupported operand types");
			break;
	}
	return op1 % ((long) zephir_get_numberval(op2));
}

/**
 * Do safe divisions between two double/zval
 */
long zephir_safe_mod_double_zval(double op1, zval *op2 TSRMLS_DC) {
	if (!zephir_get_numberval(op2)) {
		zend_error(E_WARNING, "Division by zero");
		return 0;
	}
	switch (Z_TYPE_P(op2)) {
		case IS_ARRAY:
		case IS_OBJECT:
		case IS_RESOURCE:
			zend_error(E_WARNING, "Unsupported operand types");
			break;
	}
	return (long) op1 % ((long) zephir_get_numberval(op2));
}

void zephir_ceil(zval *return_value, zval *op1 TSRMLS_DC)
{
	convert_scalar_to_number_ex(&op1);

	if (Z_TYPE_PP(&op1) == IS_DOUBLE) {
		RETURN_DOUBLE(ceil(Z_DVAL_PP(&op1)));
	} else if (Z_TYPE_PP(&op1) == IS_LONG) {
		convert_to_double_ex(&op1);
		RETURN_DOUBLE(Z_DVAL_PP(&op1));
	}
	RETURN_FALSE;
}

extern double _php_math_round(double value, int places, int mode);

void zephir_round(zval *return_value, zval *op1, zval *op2, zval *op3 TSRMLS_DC)
{
	int places = 0;
	long mode = PHP_ROUND_HALF_UP;
	double return_val;

	convert_scalar_to_number_ex(&op1);

	if (op2) {
		places = zephir_get_intval_ex(op2);
	}
	if (op3) {
		mode = zephir_get_intval_ex(op3);
	}

	switch (Z_TYPE_PP(&op1)) {
		case IS_LONG:
			/* Simple case - long that doesn't need to be rounded. */
			if (places >= 0) {
				RETURN_DOUBLE((double) Z_LVAL_PP(&op1));
			}
			/* break omitted intentionally */

		case IS_DOUBLE:
			return_val = (Z_TYPE_PP(&op1) == IS_LONG) ? (double)Z_LVAL_PP(&op1) : Z_DVAL_PP(&op1);
			return_val = _php_math_round(return_val, places, mode);
			RETURN_DOUBLE(return_val);
			break;

		default:
			RETURN_FALSE;
			break;
	}
}

#if PHP_VERSION_ID < 50600
#include "Zend/zend_multiply.h"
void zephir_pow_function_ex(zval *return_value, zval *zbase, zval *zexp TSRMLS_DC)
{
	/* make sure we're dealing with numbers */
	convert_scalar_to_number(zbase TSRMLS_CC);
	convert_scalar_to_number(zexp TSRMLS_CC);

	/* if both base and exponent were longs, we'll try to get a long out */
	if (Z_TYPE_P(zbase) == IS_LONG && Z_TYPE_P(zexp) == IS_LONG && Z_LVAL_P(zexp) >= 0) {
		long l1 = 1, l2 = Z_LVAL_P(zbase), i = Z_LVAL_P(zexp);

		if (i == 0) {
			RETURN_LONG(1L);
		} else if (l2 == 0) {
			RETURN_LONG(0);
		}

		/* calculate pow(long,long) in O(log exp) operations, bail if overflow */
		while (i >= 1) {
			int overflow;
			double dval = 0.0;

			if (i % 2) {
				--i;
				ZEND_SIGNED_MULTIPLY_LONG(l1, l2, l1, dval, overflow);
				if (overflow) RETURN_DOUBLE(dval * pow(l2, i));
			} else {
				i /= 2;
				ZEND_SIGNED_MULTIPLY_LONG(l2, l2, l2, dval,overflow);
				if (overflow) RETURN_DOUBLE((double)l1 * pow(dval, i));
			}
			if (i == 0) {
				RETURN_LONG(l1);
			}
		}
	}
	convert_to_double(zbase);
	convert_to_double(zexp);

	RETURN_DOUBLE(pow(Z_DVAL_P(zbase), Z_DVAL_P(zexp)));
}
#endif
