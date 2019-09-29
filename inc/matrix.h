/**
 * Copyright @ 深圳市谷米万物科技有限公司. 2009-2019. All rights reserved.
 * File name:        matrix.h
 * Author:           王志华       
 * Version:          1.0
 * Date:             2019-09-07
 * Description:      矩阵运算
 * Others:           
 * Function List:    
    
 * History: 
    1. Date:         2019-09-07
       Author:       王志华
       Modification: 创建初始版本
    2. Date:          
       Author:         
       Modification: 

 */

#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <gm_type.h>

typedef struct 
{
  int rows;
  int cols;

  double** data;
} Matrix;

/**
 * Function:   创建矩阵
 * Description:默认值为0.0
 * Input:	   rows——行数；cols——列数
 * Output:	   无
 * Return:	   创建好的矩阵
 * Others:	   无
 */
Matrix* matrix_create(int rows, int cols);

/**
 * Function:   销毁矩阵
 * Description:释放内存
 * Input:	   m——矩阵
 * Output:	   m——销毁后的矩阵
 * Return:	   无
 * Others:	   无
 */
void matrix_destroy(Matrix* p_m);

/**
 * Function:   设置矩阵值
 * Description:逐行进行
 * Input:	   可变参数
 * Output:	   m——输出矩阵
 * Return:	   无
 * Others:	   无
 */
void matrix_set(Matrix* p_m, ...);

/**
 * Function:   设置单位矩阵
 * Description:
 * Input:	   无
 * Output:	   m——输出矩阵
 * Return:	   无
 * Others:	   无
 */
void matrix_set_identity(Matrix* p_m);

/**
 * Function:   将矩阵source复制到destination
 * Description:
 * Input:	   source——原矩阵
 * Output:	   无
 * Return:	   无
 * Others:	   无
 */
void matrix_copy(Matrix source, Matrix* p_destination);

/**
 * Function:   将矩阵数据打印出来
 * Description:
 * Input:	   m——矩阵
 * Output:	   无
 * Return:	   无
 * Others:	   无
 */
void matrix_print(Matrix m);

/**
 * Function:   矩阵加法：a+b=c
 * Description:
 * Input:	   a——第1个加数；b——第2个加数
 * Output:	   c——结果
 * Return:	   无
 * Others:	   无
 */
void matrix_add(Matrix a, Matrix b, Matrix* p_c);

/**
 * Function:   矩阵减法
 * Description:
 * Input:	   a——被减数；b——减数
 * Output:	   c——结果
 * Return:	   无
 * Others:	   无
 */
void matrix_subtract(Matrix a, Matrix b, Matrix* p_c);

/**
 * Function:   从单位矩阵中减去a，结果存在a
 * Description:
 * Input:	   a——矩阵a
 * Output:	   a——矩阵a
 * Return:	   无
 * Others:	   无
 */
void matrix_subtract_from_identity(Matrix* p_a);

/**
 * Function:   矩阵乘法：a*b=c
 * Description:a的列要与b的行数相等，c的行数等于a的行数，c的列数等于b的列数
 * Input:	   a——第1个乘数；b——第2个乘数
 * Output:	   c——结果矩阵
 * Return:	   无
 * Others:	   无
 */
void matrix_multiply(Matrix a, Matrix b, Matrix* p_c);

/**
 * Function:   a乘以b的转置矩阵，结果放到c
 * Description:a的列要与b的列数数相等，c的行数等于a的行数，c的列数等于b的行数
 * Input:	   a——矩阵a；b——矩阵a
 * Output:	   c——结果矩阵
 * Return:	   无
 * Others:	   无
 */
void matrix_multiply_by_transpose(Matrix a, Matrix b, Matrix* p_c);

/**
 * Function:   矩阵转置
 * Description:
 * Input:	   input——输入矩阵
 * Output:	   output——输出矩阵
 * Return:	   无
 * Others:	   无
 */
void matrix_transpose(Matrix input, Matrix* p_output);

/**
 * Function:   判断两个矩阵是否近似相等
 * Description:
 * Input:	   a——矩阵a；b——矩阵a；tolerance——允许的误差
 * Output:	   无
 * Return:	   true——近似相等；false——不相等
 * Others:	   无
 */
bool matrix_equal(Matrix a, Matrix b, double tolerance);

/**
 * Function:   矩阵乘以一个标量
 * Description:
 * Input:	   m——输入矩阵；scalar——标量
 * Output:	   m——输出矩阵
 * Return:	   无
 * Others:	   无
 */
void matrix_scale(Matrix* p_m, double scalar);

/**
 * Function:   交换矩阵m的第r1和r2行
 * Description:行变换的一种
 * Input:	   m——输入矩阵；r1、r2——交换的行号
 * Output:	   m——输出矩阵
 * Return:	   无
 * Others:	   无
 */
void matrix_swap_rows(Matrix* p_m, int r1, int r2);

/**
 * Function:   矩阵的第r行乘以一个标量
 * Description:行变换的一种
 * Input:	   m——输入矩阵；r——第几行；scalar——标量
 * Output:	   m——输出矩阵
 * Return:	   无
 * Others:	   无
 */
void matrix_scale_row(Matrix* p_m, int r, double scalar);

/**
 * Function:   将矩阵的第r2行乘以一个标量再加到第r1行
 * Description:行变换的一种
 * Input:	   m——输入矩阵；r——第几行；scalar——标量
 * Output:	   m——输出矩阵
 * Return:	   无
 * Others:	   无
 */
void matrix_shear_row(Matrix* p_m, int r1, int r2, double scalar);

/**
 * Function:   求方阵的逆矩阵
 * Description:输入不一定可逆
 * Input:	   input——输入矩阵
 * Output:	   output——输出矩阵
 * Return:	   true——可逆；false——不可逆
 * Others:	   无
 */
bool matrix_destructive_invert(Matrix input, Matrix* p_output);

#endif

