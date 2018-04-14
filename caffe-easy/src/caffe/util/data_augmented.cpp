
#include "caffe/util/data_augmented.hpp"

namespace caffe {
#ifdef USE_OPENCV
	//��ͼ����
	namespace Tools{
		int seed(){
			return time(0);
		}

		template<typename T>
		T randr(T minval, T maxval)
		{
			double acc = rand() / (double)(RAND_MAX);
			if (minval > maxval) std::swap(minval, maxval);
			return (acc * (maxval - minval)) + minval;
		}
	}

	//�ṩ��ͼ�����ṩ0-1��Χ��ֵ��ͼƬ
	namespace GlobalAugmented{

		//�����������С���������ṩ��ͼȫ����float��
		void noize(Mat& img){

			cv::RNG rng(Tools::seed());
			Mat rd = img.clone();
			rng.fill(rd, RNG::UNIFORM, -1, 1);

			img = img + rd * Tools::randr(0.002, 0.05);
		}

		//���Դ
		void pointLight(Mat& img, float minLimit, float maxLimit){
			Mat mask = Mat::zeros(img.size(), CV_8U);
			float r = Tools::randr(100.0, max(img.cols, img.rows)*0.5);
			circle(mask, Point(Tools::randr(10, img.cols - 10), Tools::randr(10, img.rows - 10)), r, Scalar::all(255), -1);
			blur(mask, mask, Size(r, r));

			mask.convertTo(mask, CV_32F, 1 / 255.0);
			Mat color(img.size(), CV_32FC(img.channels()), Scalar::all(Tools::randr(-1.0, 1.0)));
			Mat maskMerge = mask;

			if (img.channels() > 1)
				cvtColor(mask, maskMerge, CV_GRAY2BGR);

			Mat t = color.mul(maskMerge);
			cv::add(img, t, img);

			vector<Mat> chs;
			split(img, chs);
			for (int i = 0; i < chs.size(); ++i){
				double mx, mi;
				cv::minMaxIdx(chs[i], &mi, &mx);
				chs[i] = (chs[i] - mi) / (mx - mi) * (maxLimit - minLimit) + minLimit;
			}
			merge(chs, img);
		}

		//��ʵ���������ͼ���ǲ����ܳ��ֳ���1��
		void limitRange(Mat& img, float minLimit, float maxLimit){
			vector<Mat> chs;
			split(img, chs);
			for (int i = 0; i < chs.size(); ++i){
				chs[i].setTo(minLimit, chs[i] < minLimit);
				chs[i].setTo(maxLimit, chs[i] > maxLimit);
			}
			merge(chs, img);
		}

		void globalLight(Mat& img){
			Mat color(img.size(), CV_32FC(img.channels()), Scalar::all(Tools::randr(-0.3, 0.3)));
			cv::add(img, color, img);
		}

		void augment(Mat& img, float minLimit, float maxLimit){
			int order[] = { 0, 1, 2 };
			int num = sizeof(order) / sizeof(order[0]);
			std::random_shuffle(order, order + num);

			//һ���ĸ���ɶ��������ֻ������
			if (Tools::randr(0.0, 1.0) < 0.1){
				noize(img);
				return;
			}

			for (int i = 0; i < num; ++i){
				switch (order[i]){
				case 0:
					pointLight(img, minLimit, maxLimit);
					break;
				case 1:
					globalLight(img);
					break;
				case 2:
					noize(img);
					break;
				}
			}
			limitRange(img, minLimit, maxLimit);
		}
	}
#endif
}  // namespace caffe
