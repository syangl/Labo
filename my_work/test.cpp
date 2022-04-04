#include<iostream>
#include<arm_neon.h>
using namespace std;

int32_t * key_splot_;
int32_t data_capacity_;

#define ALEX_DATA_NODE_KEY_AT key_splot_

template <class K>
inline int simd_search_upper_bound(int m, const K &key)
{
  int32x4_t keys = vmovq_n_s32(key);
  //cout<<vgetq_lane_s32(keys,0);cout<<vgetq_lane_s32(keys,1);cout<<vgetq_lane_s32(keys,2);cout<<vgetq_lane_s32(keys,3)<<endl;
  int32x4_t m_zero = vmovq_n_s32(0);
  int32x4_t m_load;                 //加载数组4个元素
  int32x4_t flags = vmovq_n_s32(0); // 比较结果标志
  int bound;
  if (ALEX_DATA_NODE_KEY_AT[m]>key)//预测位置大于key实际位置
  {
    //cout<<"ALEX_DATA_NODE_KEY_AT "<<ALEX_DATA_NODE_KEY_AT[m]<<" enter1"<<endl;
    for (int j = m-4; j>=0; j -= 4)
    {
      ////从[0:m-4]搜索key，每次检查大小为4的向量keys，从预测位置m开始从右至左搜索以减少搜索次数。
      ////加载数组中的四个key到m_load向量
      m_load = vld1q_s32(ALEX_DATA_NODE_KEY_AT+j);
      //cout<<"m_load "<<vgetq_lane_s32(m_load,0);cout<<vgetq_lane_s32(m_load,1);cout<<vgetq_lane_s32(m_load,2);cout<<vgetq_lane_s32(m_load,3)<<endl;
      ////比较结果标志位flags
      flags = (int32x4_t)vceqq_s32(keys, m_load);
      //cout<<"flags1 "<<vgetq_lane_s32(flags,0);cout<<vgetq_lane_s32(flags,1);cout<<vgetq_lane_s32(flags,2);cout<<vgetq_lane_s32(flags,3)<<endl;
      ////令flags向量每个元素为0或1
      flags = vsubq_s32(m_zero, flags);
      //cout<<"flags2 "<<vgetq_lane_s32(flags,0);cout<<vgetq_lane_s32(flags,1);cout<<vgetq_lane_s32(flags,2);cout<<vgetq_lane_s32(flags,3)<<endl;
      ////调换flags高两位和低两位
      int32x4_t tmp = vcombine_s32(vget_high_s32(flags), vget_low_s32(flags));
      ////flags和倒flags相加
      flags = vaddq_s32(flags, tmp);
      //cout<<"flags3 "<<vgetq_lane_s32(flags,0);cout<<vgetq_lane_s32(flags,1);cout<<vgetq_lane_s32(flags,2);cout<<vgetq_lane_s32(flags,3)<<endl;
      
      int32x4_t final_flag = vcombine_s32(vget_low_s32(flags), vget_low_s32(flags));
      ////把flags的2和3位移至0和1位，再复制给2和3位
      int32_t tmp_arr[4];
      tmp_arr[0]=tmp_arr[2]=vgetq_lane_s32(final_flag, 1);///////////////////////////////////不太好
      tmp_arr[1]=tmp_arr[3]=vgetq_lane_s32(final_flag, 2);
      tmp = vld1q_s32(tmp_arr);
      //cout<<"tmp "<<vgetq_lane_s32(tmp,0);cout<<vgetq_lane_s32(tmp,1);cout<<vgetq_lane_s32(tmp,2);cout<<vgetq_lane_s32(tmp,3)<<endl;
      ////flags和移位后的flags相加，结果为1111或0000
      final_flag = vaddq_s32(final_flag, tmp);
      //cout<<"flags4 "<<vgetq_lane_s32(final_flag,0);cout<<vgetq_lane_s32(final_flag,1);cout<<vgetq_lane_s32(final_flag,2);cout<<vgetq_lane_s32(final_flag,3)<<endl;
      ////res标志是否找到key
      int32_t res = vgetq_lane_s32(final_flag, 3);
      //cout<<"res "<<res<<endl;
      if (res == 1)
      {////key在此向量中，确定其具体位置
        for (int i = j; i < j + 4; i++)
        {
          if (ALEX_DATA_NODE_KEY_AT[i]==key)
          {
            //cout<<"ret "<<i<<endl;
            return bound = i;
          }
        }
      }
    }

    for (int i = 0; i < 3; i++)
    {
      if (ALEX_DATA_NODE_KEY_AT[i]==key)
      {
        return bound = i;
      }
    }
  }
  else
  {
    //cout<<"ALEX_DATA_NODE_KEY_AT "<<ALEX_DATA_NODE_KEY_AT[m]<<" enter2"<<endl;
    for (int j = m; j < data_capacity_; j += 4)
    {

      m_load = vld1q_s32(ALEX_DATA_NODE_KEY_AT + j);
      flags = (int32x4_t)vceqq_s32(keys, m_load);
      flags = vsubq_s32(m_zero, flags);
      int32x4_t tmp = vcombine_s32(vget_high_s32(flags), vget_low_s32(flags));
      flags = vaddq_s32(flags, tmp);
      int32x4_t final_flag = vcombine_s32(vget_low_s32(flags), vget_low_s32(flags));
      int32_t tmp_arr[4];
      tmp_arr[0]=tmp_arr[2]=vgetq_lane_s32(final_flag, 1);
      tmp_arr[1]=tmp_arr[3]=vgetq_lane_s32(final_flag, 2);
      tmp = vld1q_s32(tmp_arr);
      final_flag = vaddq_s32(final_flag, tmp);
      int32_t res = vgetq_lane_s32(final_flag, 3);
      if (res == 1)
      {
        for (int i = j; i < j + 4; i++)
        {
          if (ALEX_DATA_NODE_KEY_AT[i]==key)
          {
            return bound = i;
          }
        }
      }
    }

    for (int i = data_capacity_ - 3; i < data_capacity_; i++)
    {
      if (ALEX_DATA_NODE_KEY_AT[i]==key)
      {
        return bound = i;
      }
    }
  }
  return 0;
}

int main(){//简单测试
  data_capacity_ = 1000;
	ALEX_DATA_NODE_KEY_AT = new int [data_capacity_];
  for(int i = 0; i < data_capacity_; i++){
    ALEX_DATA_NODE_KEY_AT[i] = i*2;
  }
  cout<<ALEX_DATA_NODE_KEY_AT[simd_search_upper_bound(1000,1998)]<<endl;
	return 0;
}
