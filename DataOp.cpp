#include "DataOp.h"



DataFilter::DataFilter()
{
	outputData = new UDPData;
	memset(UDPOriFrame,0,1024);
	outputData->frameLength[0] = 2;
	outputData->frameLength[1] = 165;
	outputData->frameOther[0] = 4;
	outputData->frameOther[1] = 3;
	outputData->frameOther[2] = 162;
}

DataFilter::~DataFilter()
{
	if (sizeof(*outputData)!=0)
	{
		delete outputData;
		outputData = NULL;
	}
}


bool DataFilter::getData()
{
	if (UDPFrameDec[0] == 250 && 
		UDPFrameDec[4] == outputData->frameOther[0] &&
		UDPFrameDec[5] == outputData->frameOther[1] &&
		UDPFrameDec[6] == outputData->frameOther[2] &&
		UDPFrameDec[681] == 251)
	{
		outputData->frameHead = UDPFrameDec[0];
		outputData->frameIndex = UDPFrameDec[1];
		outputData->frameUpdate = UDPFrameDec[7];
		outputData->frame = UDPFrameDec[8];
		outputData->frameRate = UDPFrameDec[10];
		for (int i=0;i<6;i++)
		{
			outputData->rootPos[i] = UDPFrameDec[i + 11];
		}

		for (int i=0;i<20;i++)
		{
			outputData->RHandState[i] = UDPFrameDec[i + 224];
		}
		for (int i=0;i<160;i++)
		{
			outputData->RHandRot[i] = UDPFrameDec[i + 244];
		}
		for (int i=0;i<20;i++)
		{
			outputData->LHandState[i] = UDPFrameDec[i + 404];
		}
		for (int i=0;i<160;i++)
		{
			outputData->LHandRot[i] = UDPFrameDec[i + 424];
		}
		for (int i=0;i<3;i++)
		{
			outputData->frameEndOther[i] = UDPFrameDec[i + 584];
		}
		outputData->frameTail = UDPFrameDec[681];
		outputData->frameCor = UDPFrameDec[682];
		
		return true;
	}
	else {
		return false;
	}
}

bool DataFilter::transData()
{

	stringstream ss;
	string s;
	int tempNum;
	for (int j = 0; j < 683; j++)
	{
		ss<<std::setw(2)<<std::setfill('0')<<std::hex<<(static_cast<int>(UDPOriFrame[j]) & 0xff);
		ss>>tempNum;
		UDPFrameDec.push_back(tempNum);
		ss.clear();
	}

	return true;

}

bool DataFilter::dataOpt()
{
// 	__try
// 	{
	double tempList[3] = { 0 };
	int j = 0;
	int k = 0;
	for (size_t i = 0; i < 20; i++)
	{
		if (i == 4 || i == 8 || i == 12 || i == 16)
		{
			i++;
		}
		double w = (outputData->RHandRot[i*8] * 256 + outputData->RHandRot[i*8 + 1]) * 0.0001f;
		double x = (outputData->RHandRot[i*8 + 2] * 256 + outputData->RHandRot[i*8 + 3]) * 0.0001f;
		double y = (outputData->RHandRot[i*8 + 4] * 256 + outputData->RHandRot[i*8 + 5]) * 0.0001f;
		double z = (outputData->RHandRot[i*8 + 6] * 256 + outputData->RHandRot[i*8 + 7]) * 0.0001f;



		memset(tempList, 0, 3);
		quatToEuler2(w, x, y, z, tempList);




		memcpy(&RRotList[j][0], &tempList[0], 8);
		memcpy(&RRotList[j][1], &tempList[1], 8);
		memcpy(&RRotList[j][2], &tempList[2], 8);
		j++;

	}
	for (size_t i = 0; i < 20; i++)
	{
			

		if (i==4||i==8||i==12||i==16)
		{
			i++;
		}
		double w = (outputData->LHandRot[i*8] * 256 + outputData->LHandRot[i*8 + 1]) * 0.0001f;
		double x = (outputData->LHandRot[i*8 + 2] * 256 + outputData->LHandRot[i*8 + 3]) * 0.0001f;
		double y = (outputData->LHandRot[i*8 + 4] * 256 + outputData->LHandRot[i*8 + 5]) * 0.0001f;
		double z = (outputData->LHandRot[i*8 + 6] * 256 + outputData->LHandRot[i*8 + 7]) * 0.0001f;


// 		vector<float> LRot;
// 		string s2 = "LHandRot";
// 
// 		LRot.push_back(outputData->LHandRot[i*8]);
// 		LRot.push_back(outputData->LHandRot[i*8+1]);
// 		LRot.push_back(outputData->LHandRot[i*8+2]);
// 		LRot.push_back(outputData->LHandRot[i*8+3]);
// 		LRot.push_back(outputData->LHandRot[i*8+4]);
// 		LRot.push_back(outputData->LHandRot[i*8+5]);
// 		LRot.push_back(outputData->LHandRot[i*8+6]);
// 		LRot.push_back(outputData->LHandRot[i*8+7]);
// 
// 		saveStr(s2,"G:\\test\\device_Data.txt");
// 		saveVec(LRot,"G:\\test\\device_Data.txt");


		memset(tempList, 0, 3);
		quatToEuler2(w, x, y, z, tempList);


// 		string s = "LHandRot";
// 		vector<double> sV;
// 		sV.push_back(tempList[0]);
// 		sV.push_back(tempList[1]);
// 		sV.push_back(tempList[2]);
// 		saveStr(s,"G:\\test\\device_Data2.txt");
// 		saveVec(sV,"G:\\test\\device_Data2.txt");

		memcpy(&LRotList[k][0], &tempList[0], 8);
		memcpy(&LRotList[k][1], &tempList[1], 8);
		memcpy(&LRotList[k][2], &tempList[2], 8);
		k++;
	}


	return true;
//	}
// 	__except(EXCEPTION_EXECUTE_HANDLER)
// 	{
// 		return false;
// 	}

}

int DataFilter::strToInt(char* InData)
{
	int ret = 0;
	double as = 48;
	int len = strlen(InData);
	for (int i = 0; i < len; i++)
	{
		ret += (InData[i] - as) * (pow(10, (len - i - 1)));
	}
	return ret;
}

void DataFilter::quatToEuler(double w,double x,double y,double z, double* rotList)
{
	float sp = -2.0f * (y * z - w * x);

	if (fabs(sp)>0.9999f)
	{
		rotList[1] = 1.570796f * sp;
		rotList[2] = atan2(-x * z + w * y, 0.5f - y * y - z * z);
		rotList[0] = 0.0f;
	}
	else
	{
		rotList[1] = asin(sp);
		rotList[0] = atan2(x * z + w * y, 0.5f - x * x - y * y);
		rotList[2] = atan2(x * y + w * z, 0.5f - x * x - z * z);
	}
}

void DataFilter::quatToEuler1(double w,double x,double y,double z, double* rotList)
{
	const double Epsilon = 0.0009765625f;
	const double Threshold = 0.5f - Epsilon;
	double Test = w*y - x*z;
	if(Test<-Threshold||Test>Threshold)
	{
		int si = sign(Test);
		rotList[2] = -2*si*(double)atan2(x,w);
		rotList[1] = si*(PI/2.0);
		rotList[0] = 0;
	}
	else
	{
		rotList[0] = atan2(2*(y*z + w*x),w*w - x*x - y*y + z*z);
		rotList[1] = asin(-1*(x*z - w*y));
		rotList[2] = atan2(2*(x*y + w*z),w*w + x*x - y*y - z*z);
	}

}

int DataFilter::sign(double x)
{
	if (x==0)
	{
		return 0;
	}
	else if(x>0)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

void DataFilter::checkData()
{

}

void DataFilter::quatToEuler2(double w, double x, double y, double z, double* rotList)
{
	double theta = x*x + y*y + z*z;
	if (theta>0.0f)
	{
		double sin_theta = sqrt(theta);
		double cos_theta = w;

		double two_theta = 2.0*((cos_theta<0.0)?atan2(-sin_theta,-cos_theta):atan2(sin_theta,cos_theta));

		double k = two_theta/sin_theta;

		rotList[0] = x*k;
		rotList[1] = y*k;
		rotList[2] = z*k;
	}
	else
	{
		rotList[0] = x*2.0f;
		rotList[1] = y*2.0f;
		rotList[2] = z*2.0f;
	}
}




