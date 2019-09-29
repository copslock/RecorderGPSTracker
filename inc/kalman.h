/**
 * Copyright @ 深圳市谷米万物科技有限公司. 2009-2019. All rights reserved.
 * File name:        kalman.h
 * Author:           王志华       
 * Version:          1.0
 * Date:             2019-09-07
 * Description:      卡尔曼滤波算法
 * Others:           不支持控制输入
 * Function List:    
    
 * History: 
    1. Date:         2019-09-07
       Author:       王志华
       Modification: 创建初始版本
    2. Date:          
       Author:         
       Modification: 

 */

#ifndef __KALMAN_H__
#define __KALMAN_H__

#include "matrix.h"

typedef struct 
{
  //卡尔曼模型中的k
  int timestep;

  //状态转移矩阵和观测矩阵的维度
  int state_dimension, observation_dimension;
  
  //状态转移矩阵F_k
  Matrix* p_state_transition;
  
  //观测矩阵H_k
  Matrix* p_observation_model;
  
  //状态转移噪声矩阵Q_k
  Matrix* p_process_noise_covariance;
  
  //观测噪声矩阵R_k
  Matrix* p_observation_noise_covariance;

  //观测值z_k，每一步由用户修改
  Matrix* p_observation;
  
  //预测状态x-hat_k|k-1，每一步由滤波器更新
  Matrix* p_predicted_state;
  
  //预测估计协方差P_k|k-1
  Matrix* p_predicted_estimate_covariance;
  
  //新息（innovation，y-tilde_k）
  Matrix* p_innovation;
  
  //新息协方差（S_k）
  Matrix* p_innovation_covariance;
  
  //新息协方差逆矩阵（S_k^-1）
  Matrix* p_inverse_innovation_covariance;
  
  // 最优卡尔曼增益(K_k)
  Matrix* p_optimal_gain;
  
  //状态估计（x-hat_k|k）
  Matrix* p_state_estimate;
  
  //估计协方差（P_k|k）
  Matrix* p_estimate_covariance;

  //存储中间计算结果
  Matrix* p_vertical_scratch;
  
  Matrix* p_small_square_scratch;
  
  Matrix* p_big_square_scratch;
  
} KalmanFilter;

/**
 * Function:   创建kalman模块
 * Description:创建kalman模块
 * Input:      state_dimension——状态转移矩阵维度，observation_dimension——观测矩阵维度
 * Output:     无
 * Return:     非空——成功；NULL——失败
 * Others:     使用前必须调用,否则调用其它接口返回失败错误码
 */
KalmanFilter* kalman_create(int state_dimension,int observation_dimension);

/**
 * Function:   销毁kalman模块
 * Description:销毁kalman模块
 * Input:      filter——滤波器
 * Output:     无
 * Return:     无
 * Others:       
 */
void kalman_destroy(KalmanFilter* p_filter);

/**
 * Function:   更新函数
 * Description:运行预测和估计，在运行这一步之前要先设置观测值，并且要定义好以下变量：
               f.state_transition
               f.observation_model
               f.process_noise_covariance
               f.observation_noise_covariance
               也建议使用合理的猜想初始化以下变量：
 			   f.state_estimate
               f.estimate_covariance
 * Input:	   filter——滤波器
 * Output:	   无
 * Return:	   非空——成功；NULL——失败
 * Others:	   
 */
bool kalman_update(KalmanFilter* p_filter);

/**
 * Function:   update函数的预测
 * Description:update函数的预测
 * Input:      filter——滤波器
 * Output:     无
 * Return:     无
 * Others:       
 */
void kalman_predict(KalmanFilter* p_filter);

/**
 * Function:   update函数的估计
 * Description:update函数的估计
 * Input:      filter——滤波器
 * Output:     无
 * Return:     无
 * Others:       
 */
bool kalman_estimate(KalmanFilter* p_filter);

#endif

