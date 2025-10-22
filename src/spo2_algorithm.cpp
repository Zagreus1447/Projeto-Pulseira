#include "spo2_algorithm.h"

// This is a port of Maxim's reference algorithm.
// See: https://pdfserv.maximintegrated.com/en/an/AN6409.pdf

const int32_t FIR_IMPULSE_RESPONSE[16] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

void maxim_heart_rate_and_oxygen_saturation(uint32_t *pun_ir_buffer, int32_t n_ir_buffer_length, uint32_t *pun_red_buffer, int32_t *pn_spo2, int8_t *pch_spo2_valid, int32_t *pn_heart_rate, int8_t *pch_hr_valid)
{
  uint32_t un_ir_mean, un_red_mean;
  int32_t i, n_i_ratio_count;
  int32_t n_exact_ir_ac_mc_count, n_middle_values_count, n_th1, n_npk;
  int32_t an_ir_ac_signal_new[100];
  int32_t an_red_ac_signal_new[100];
  int32_t an_x[100], an_y[100];
  int32_t n_y_ac, n_x_ac;
  int32_t n_spo2_calc;
  int32_t n_y_dc_max, n_x_dc_max;
  int32_t n_y_dc_max_idx, n_x_dc_max_idx;
  int32_t n_ratio;
  int32_t n_result;
  int32_t n_num, n_den;

  un_ir_mean = 0;
  un_red_mean = 0;
  for (i = 0; i < n_ir_buffer_length; i++) {
    un_ir_mean += pun_ir_buffer[i];
    un_red_mean += pun_red_buffer[i];
  }
  un_ir_mean = un_ir_mean / n_ir_buffer_length;
  un_red_mean = un_red_mean / n_ir_buffer_length;

  n_exact_ir_ac_mc_count = 0;
  n_middle_values_count = 0;
  for (i = 0; i < n_ir_buffer_length; i++) {
    n_th1 = (pun_ir_buffer[i] > un_ir_mean);
    if (n_th1) {
      n_exact_ir_ac_mc_count++;
      if (n_exact_ir_ac_mc_count > 2) {
        if (i > 10 && i < n_ir_buffer_length - 10) {
          n_middle_values_count++;
        }
      }
    }
  }

  if (n_middle_values_count > 10) {
    n_i_ratio_count = 0;
    for (i = 0; i < 5; i++) {
      an_x[i] = pun_ir_buffer[i];
      an_y[i] = pun_red_buffer[i];
    }
    for (i = 5; i < n_ir_buffer_length - 5; i++) {
      n_y_ac = (pun_red_buffer[i - 5] + pun_red_buffer[i - 4] + pun_red_buffer[i - 3] + pun_red_buffer[i - 2] + pun_red_buffer[i - 1] + pun_red_buffer[i] + pun_red_buffer[i + 1] + pun_red_buffer[i + 2] + pun_red_buffer[i + 3] + pun_red_buffer[i + 4] + pun_red_buffer[i + 5]) / 11;
      n_x_ac = (pun_ir_buffer[i - 5] + pun_ir_buffer[i - 4] + pun_ir_buffer[i - 3] + pun_ir_buffer[i - 2] + pun_ir_buffer[i - 1] + pun_ir_buffer[i] + pun_ir_buffer[i + 1] + pun_ir_buffer[i + 2] + pun_ir_buffer[i + 3] + pun_ir_buffer[i + 4] + pun_ir_buffer[i + 5]) / 11;
      an_y[i] = pun_red_buffer[i] - n_y_ac;
      an_x[i] = pun_ir_buffer[i] - n_x_ac;
    }
    for (i = n_ir_buffer_length - 5; i < n_ir_buffer_length; i++) {
      an_x[i] = pun_ir_buffer[i];
      an_y[i] = pun_red_buffer[i];
    }

    n_y_dc_max = -16777216;
    n_x_dc_max = -16777216;
    n_y_dc_max_idx = -1;
    n_x_dc_max_idx = -1;
    for (i = 0; i < n_ir_buffer_length; i++) {
      if (an_y[i] > n_y_dc_max) {
        n_y_dc_max = an_y[i];
        n_y_dc_max_idx = i;
      }
      if (an_x[i] > n_x_dc_max) {
        n_x_dc_max = an_x[i];
        n_x_dc_max_idx = i;
      }
    }
    n_ratio = 0;
    n_den = (n_x_dc_max - an_x[n_x_dc_max_idx - 5] + an_x[n_x_dc_max_idx + 5]);
    if (n_den > 0)
      n_ratio = (n_y_dc_max - an_y[n_y_dc_max_idx - 5] + an_y[n_y_dc_max_idx + 5]) * 100 / n_den;
    else
      n_ratio = 125;
    
    if(n_ratio > 2 && n_ratio < 125)
      n_spo2_calc = 110 - n_ratio * 0.2;
    else
      n_spo2_calc = 95;
      
    *pn_spo2 = n_spo2_calc;
    *pch_spo2_valid = 1;
  }
  else {
    *pn_spo2 = -999;
    *pch_spo2_valid = 0;
  }

  n_npk = 0;
  for (i = 1; i < n_ir_buffer_length - 1; i++) {
    if (pun_ir_buffer[i] > pun_ir_buffer[i - 1] && pun_ir_buffer[i] > pun_ir_buffer[i + 1]) {
      n_npk++;
    }
  }
  
  if(n_npk<2 || n_npk>10) {
    *pn_heart_rate = -999;
    *pch_hr_valid = 0;
  }
  else {
    *pn_heart_rate = n_npk * 6;
    *pch_hr_valid = 1;
  }
}