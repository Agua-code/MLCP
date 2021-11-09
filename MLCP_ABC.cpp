// MLCP_ABC.cpp : �������̨Ӧ�ó������ڵ㡣
//ABC

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <time.h>
#include <ctime>
#include <string>
#include <string.h>
#include <algorithm>
#include <vector>

//�ð汾������swap������tt�̶�15��alpha0.01��omega1000����exp���omega���ˣ�tt����
using namespace std;
#define MAX_VAL 999999

clock_t startTime, endTime;//��¼����ʱ��
double bestTime;//���ʱ��
double cutting_time = 180;//��ֹ����
//string inFile = "C:/Users/18367/Documents/Visual Studio 2015/Projects/MLCP/Debug/data/le450_15c.col";

string inFile = "C:/Users/18367/Documents/Visual Studio 2015/Projects/MLCP/Debug/data/";
string outFile = "F:/��������/����1/ABC/180/";

vector<string> fileList = {
	"anna.col",
	"david.col",
	"queen10_10.col",
	"queen13_13.col",
	"queen15_15.col",
	"queen16_16.col",
	"myciel6.col",
	"homer.col",
	"mulsol.i.5.col",
	"inithx.i.1.col",
	"inithx.i.2.col",
	"inithx.i.3.col",
	"latin_square_10.col",
	"le450_5a.col",
	"le450_5b.col",
	"le450_5c.col",
	"le450_5d.col",
	"le450_15a.col",
	"le450_15b.col",
	"le450_15c.col",
	"le450_15d.col",
	"le450_25a.col",
	"le450_25b.col",
	"le450_25c.col",
	"le450_25d.col"
};


int timeFlag = 0;
//��Ҫ���ڵĲ���
int tt = 10;//���ɳ���
float pertubationDepth = 0.01;//�Ŷ�ǿ��
int restartThreshold = 10;//����restart����ֵ
int pertubationThreshold = 500;//�����Ŷ�����ֵ
int noimprove = 0;//����δ���µĴ���
int pertubationTime = 0;//С���Ŷ��Ĵ���
float alpha = 0.1;
float beta = 0.2;//�������ֵ��Ҫ��ʵ��
float gamma1 = 0.3;
const int NP = 40;//��Ⱥ�Ĺ�ģ�����۷�+�۲��
const int FoodNumber = NP / 2;//ʳ���������Ϊ���۷������
const int maxCycle = 10000;//ֹͣ����
const int limit = 20;//�޶ȣ���������޶�û�и��²��۷�������


int edgeNum, verNum;
int** edge;//������Ƿ��б�
int** adj;//
int* adjlen;//

			//��Ҫ���ݽṹ
int** color;
int* v2color;//��ĵ�ǰ��ɫ
int* currentBestColor;//��ǰ������ɫ
int* bestColor;//ȫ��������ɫ
int* initialColor;//��ǰ��ɫ
int* lastColor;//������ɫ
			   //��Ե�ǰ������ݽṹ
int* colorlen;
int* v2idx;
int* conect;//��������ɫ�ı�
int* bian;//������ɫ�бߵ�����
		  //restart���ݽṹ
float** p;


//�ֲ�����
int* tabu;//���ɱ�
int iter = 0;//��������
int pro = 50;//ѡ������ĸ���

//��������


int Sblist[10];
double timelist[10];

struct BeeGroup
{
	int code[1000];//������ά��0-1
	int bian[2];
	int conect[1000];
	int trueFit;//��¼Ŀ�꺯��ֵ
	double rfitness;//�����Ӧֵ����
	int trail;//��ʾʵ��Ĵ�����������limit���Ƚ�
}Bee[FoodNumber];

BeeGroup NectarSource[FoodNumber];//��Դ��ע�⣺һ�е��޸Ķ��������Դ���Ե�
BeeGroup EmployedBee[FoodNumber];//���۷�
BeeGroup OnLooker[FoodNumber];//�۲��
BeeGroup BestSource;//��¼�����Դ

//�������һ�� 0-(n-1) ������
int random_int(int n)
{
	return rand() % n;
}

/*****������ʵ��****/
double random(double start, double end)//������������ڵ������
{
	return start + (end - start)*rand() / (RAND_MAX + 1.0);
}

//��ȡ����
void read_initial(string inFile)
{
	int i, j, k;
	ifstream FIC;
	FIC.open(inFile);//��ȡ����
	if (FIC.fail())
	{
		cout << "### Erreur open, File_Name " << inFile << endl;
		getchar();
		exit(0);
	}
	if (FIC.eof())
	{
		cout << "### Error open, File_Name " << inFile << endl;
		getchar();
		exit(0);
	}
	FIC >> verNum >> edgeNum;
	edge = new int*[verNum];//��¼������������
	adj = new int*[verNum];//��¼������Щ������
	color = new int*[2];//��¼ÿ����ɫ������Щ��
	p = new float*[verNum];
	adjlen = new int[verNum];//������Щ�������ĳ���
	tabu = new int[verNum];
	v2color = new int[verNum];
	currentBestColor = new int[verNum];
	bestColor = new int[verNum];
	initialColor = new int[verNum];
	lastColor = new int[verNum];
	v2idx = new int[verNum];
	conect = new int[verNum];
	colorlen = new int[2];//������ɫ��ɫ�ĵ�������
	bian = new int[2];
	color[0] = new int[verNum];
	color[1] = new int[verNum];


	for (i = 0; i < verNum; i++)
	{
		p[i] = new float[2];
		edge[i] = new int[verNum];
		adj[i] = new int[verNum];
		adjlen[i] = 0;
	}
	for (i = 0; i < verNum; i++)
		for (j = 0; j < verNum; j++)
		{
			edge[i][j] = 0;
			adj[i][j] = 0;
		}

	int a, b;
	for (i = 0; i < edgeNum; i++)
	{
		FIC >> a >> b;
		a--;
		b--;
		edge[a][b] = edge[b][a] = 1;//��������б�����

	}

	edgeNum = 0;
	for (i = 0; i < verNum - 1; i++)
		for (j = i + 1; j < verNum; j++)
			if (edge[i][j])
			{
				edgeNum++;
				adj[i][adjlen[i]] = j;
				adj[j][adjlen[j]] = i;
				adjlen[i]++;
				adjlen[j]++;
			}

	FIC.close();//�ر�����
}

double calculationTruefit(BeeGroup bee)//������ʵ�ĺ���ֵ
{
	int i, j, k;
	bee.bian[0] = 0;
	bee.bian[1] = 0;
	for (i = 0; i < verNum; i++)
	{
		bee.conect[i] = 0;
	}
	/******���Ժ���1******/
	for (i = 0; i < verNum; i++)
		for (j = 0; j < verNum; j++)
			if (edge[i][j])
				if (bee.code[i] == bee.code[j])//�������Լ���ɫ��ͬ���б������ģ��ͼ���1
					bee.conect[i]++;
	for (i = 0; i < verNum; i++)
	{
		k = bee.code[i];
		bee.bian[k] += bee.conect[i];//�ۼӵĻ���������Σ���Ҫ����2
	}
	bee.bian[0] /= 2;
	bee.bian[1] /= 2;
	int Sc = min(bee.bian[0], bee.bian[1]);

	return Sc;
}

//���������ʼ��
void random_initial()
{
	int i, j, k, m;
	for (i = 0; i < verNum; i++)
	{
		k = random_int(2);
		v2color[i] = k;//�����ɫ
	}
}

void Initilize()
{
	int i, j;
	for (i = 0; i<FoodNumber; i++)//����FoodNumber����Ⱥ
	{
		random_initial();
		for (j = 0; j<verNum; j++)//
		{
			NectarSource[i].code[j] = v2color[j];
			EmployedBee[i].code[j] = NectarSource[i].code[j];//���۷�
			OnLooker[i].code[j] = NectarSource[i].code[j];//�۲��
			BestSource.code[j] = NectarSource[0].code[j];//�����Դ
		}
		/****��Դ�ĳ�ʼ��*****/
		NectarSource[i].trueFit = calculationTruefit(NectarSource[i]);
		NectarSource[i].rfitness = 0;
		NectarSource[i].trail = 0;
		/****���۷�ĳ�ʼ��*****/
		EmployedBee[i].trueFit = NectarSource[i].trueFit;
		EmployedBee[i].rfitness = NectarSource[i].rfitness;
		EmployedBee[i].trail = NectarSource[i].trail;
		/****�۲��ĳ�ʼ��****/
		OnLooker[i].trueFit = NectarSource[i].trueFit;
		OnLooker[i].rfitness = NectarSource[i].rfitness;
		OnLooker[i].trail = NectarSource[i].trail;
	}
	/*****������Դ�ĳ�ʼ��*****/
	BestSource.trueFit = NectarSource[0].trueFit;
	BestSource.rfitness = NectarSource[0].rfitness;
	BestSource.trail = NectarSource[0].trail;
}

//��ת����
BeeGroup FLIP(BeeGroup bee, int n)
{
	int org_color;
	int org_idx;
	int i, j, k, m;
	org_color = bee.code[n];//�ŵ���ɫ,��Ե�ǰ��
	bee.code[n] = 1 - org_color;
	bee.bian[org_color] -= bee.conect[n];
	bee.conect[n] = adjlen[n] - bee.conect[n];//����
	bee.bian[1 - org_color] += bee.conect[n];//�ı�����bian

	//����conect
	for (i = 0; i < adjlen[n]; i++)//�͵������ӵĵ��б仯
	{
		k = adj[n][i];
		m = bee.code[k];
		if (m == org_color)//���ϵ���ɫһ��
			bee.conect[k]--;
		else//���ϵ���ɫ��һ��
			bee.conect[k]++;
	}
	return bee;
}


//�ֲ�����
BeeGroup local_search(BeeGroup bee)
{
	int delta;
	int temp_delta = -MAX_VAL;
	int swap_delta = -MAX_VAL;
	int temp_delta_2 = -MAX_VAL;
	int swap_delta_2 = -MAX_VAL;
	int i, j, k, m;
	int mark = -1;
	int swap_0 = -1;
	int swap_1 = -1;
	int org_0, org_1;
	int num1 = 0, num2 = 0;
	int Sc = bee.trueFit;
	//cout << pro << endl;
	for (i = 0; i < verNum; i++)
	{
		k = bee.bian[1 - bee.code[i]] + adjlen[i] - bee.conect[i];//�������һ�������ټ��㷽����������֮���������еĵ㣬��ȥ���Լ���ɫԭ����ͬ�ĵ㣬ʣ�µľ������Լ���ɫԭ����ͬ�ĵ�
		m = bee.bian[bee.code[i]] - bee.conect[i];//�Ƴ�ȥ����һ������ȥ���Լ���ɫԭ����ͬ�ĵ�
		delta = min(k, m) - Sc;//����delta
		if (delta > temp_delta)//�������flip����
		{
			temp_delta = delta;
			mark = i;
		}
	}
	if (mark == -1)
	{
		//cout << "mark==-1" << endl;
		//getchar();
	}
	else {
		bee = FLIP(bee, mark);//�����ݽṹ���и���
	}

	return bee;
}


void show_result()
{
	int i, j, k, m;

	cout << "��ɫ0[" << colorlen[0] << "]bian0[" << bian[0] << "]: ";
	for (i = 0; i < colorlen[0]; i++)
	{
		cout << color[0][i] + 1 << "[" << conect[color[0][i]] << "] ";
	}
	cout << endl << "��ɫ1[" << colorlen[1] << "]bian1[" << bian[1] << "]: ";
	for (i = 0; i < colorlen[1]; i++)
	{
		cout << color[1][i] + 1 << "[" << conect[color[1][i]] << "] ";
	}
	cout << endl;

}


void sendEmployedBees()//�޸Ĳ��۷�ĺ���
{
	int i, j, k;
	double Rij;//[-1,1]֮��������
	for (i = 0; i<FoodNumber; i++)
	{
		for (j = 0; j<verNum; j++)
		{
			EmployedBee[i].code[j] = NectarSource[i].code[j];
		}

		EmployedBee[i].trueFit = calculationTruefit(EmployedBee[i]);

		EmployedBee[i] = local_search(EmployedBee[i]);

		/*******���۷�ȥ������Ϣ*******/
		EmployedBee[i].trueFit = calculationTruefit(EmployedBee[i]);

		//cout << i + 1 << " " << EmployedBee[i].trueFit << endl;
		/******̰��ѡ�����*******/
		if (EmployedBee[i].trueFit > NectarSource[i].trueFit)
		{
			for (j = 0; j<verNum; j++)
			{
				NectarSource[i].code[j] = EmployedBee[i].code[j];
			}
			NectarSource[i].trail = 0;
			NectarSource[i].trueFit = EmployedBee[i].trueFit;
		}
		else
		{
			NectarSource[i].trail++;
		}
	}
}

void sendOnlookerBees()
{
	int i, j, t, k;
	double R_choosed;//��ѡ�еĸ���
	i = 0;
	t = 0;
	while (t < FoodNumber)
	{
		R_choosed = random(0, 1);
		//cout << R_choosed << " " << NectarSource[i].rfitness << endl;
		if (R_choosed < NectarSource[i].rfitness)//���ݱ�ѡ��ĸ���ѡ��
		{
			t++;
			/******ѡȡ������i��k********/
			for (j = 0; j<verNum; j++)
			{
				OnLooker[i].code[j] = NectarSource[i].code[j];
			}

			/****����****/

			OnLooker[i].trueFit = calculationTruefit(OnLooker[i]);

			//cout << i + 1 << " " << OnLooker[i].trueFit << endl;

			OnLooker[i] = local_search(OnLooker[i]);

			OnLooker[i].trueFit = calculationTruefit(OnLooker[i]);

			/****̰��ѡ�����******/
			if (OnLooker[i].trueFit > NectarSource[i].trueFit)
			{
				for (j = 0; j<verNum; j++)
				{
					NectarSource[i].code[j] = OnLooker[i].code[j];
				}
				NectarSource[i].trail = 0;
				NectarSource[i].trueFit = OnLooker[i].trueFit;
			}
			else
			{
				NectarSource[i].trail++;
			}
		}
		i++;
		if (i == FoodNumber)
		{
			i = 0;
		}
	}
}

void CalculateProbabilities()//�������̶ĵ�ѡ�����
{
	int i;
	double maxfit;
	maxfit = NectarSource[0].trueFit;
	for (i = 1; i < FoodNumber; i++)
	{
		if (NectarSource[i].trueFit > maxfit)
			maxfit = NectarSource[i].trueFit;
	}

	for (i = 0; i < FoodNumber; i++)
	{
		NectarSource[i].rfitness = (0.8*(NectarSource[i].trueFit / maxfit)) + 0.1;
	}
}

void MemorizeBestSource()//�������ŵ���Դ
{
	int i, j;
	for (i = 1; i<FoodNumber; i++)
	{
		if (NectarSource[i].trueFit > BestSource.trueFit)
		{
			for (j = 0; j<verNum; j++)
			{
				BestSource.code[j] = NectarSource[i].code[j];
			}
			BestSource.trueFit = NectarSource[i].trueFit;
			endTime = clock();//��ʱ
			bestTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
			//cout << "BestSource = " << BestSource.trueFit << " bestTime: " << bestTime  << "s" << endl;
		}
	}
}

BeeGroup local_search_Scout(BeeGroup bee)
{
	int delta;
	int temp_delta = -MAX_VAL;
	int swap_delta = -MAX_VAL;
	int temp_delta_2 = -MAX_VAL;
	int swap_delta_2 = -MAX_VAL;
	int i, j, k, m;
	int mark = -1;
	int swap_0 = -1;
	int swap_1 = -1;
	int org_0, org_1;
	int num1 = 0, num2 = 0;
	int Sc = bee.trueFit;
	//cout << pro << endl;
	for (i = 0; i < verNum; i++)
	{
		if (bee.code[i] == 0) {
			k = bee.bian[1 - bee.code[i]] + adjlen[i] - bee.conect[i];//�������һ�������ټ��㷽����������֮���������еĵ㣬��ȥ���Լ���ɫԭ����ͬ�ĵ㣬ʣ�µľ������Լ���ɫԭ����ͬ�ĵ�
			m = bee.bian[bee.code[i]] - bee.conect[i];//�Ƴ�ȥ����һ������ȥ���Լ���ɫԭ����ͬ�ĵ�
			delta = min(k, m) - Sc;//����delta
			if (delta > temp_delta)//�������flip����
			{
				temp_delta = delta;
				mark = i;
			}
		}
	}
	if (mark == -1)
	{
		//cout << "mark==-1" << endl;
		//getchar();
	}
	else {
		bee = FLIP(bee, mark);//�����ݽṹ���и���
	}

	return bee;
}

/*******ֻ��һֻ����**********/
void sendScoutBees()//�ж��Ƿ�������ĳ��֣���������������Դ
{
	int i, j;
	for (i = 1; i<FoodNumber; i++)
	{
		if (NectarSource[i].trail >= limit)
		{
			/*******���³�ʼ��*********/
			NectarSource[i] = local_search_Scout(NectarSource[i]);
			NectarSource[i].trail = 0;
			NectarSource[i].trueFit = calculationTruefit(NectarSource[i]);
		}
	}
}

//������
int main(int argc, char** argv)
{
	int i, j;
	/*if (argc == 3)
	{
	inFile = argv[1];
	cutting_time = atoi(argv[2]);
	}
	else
	{
	cout << "error: MLCPexp inFile time" << endl;
	getchar();
	exit(0);
	}*/
	//string fileName = argv[1];
	//string outputFileName = argv[2];
	for (int ll = 0; ll < 25; ll++) {

		string fileName = inFile + fileList[ll];
		string outputFileName = outFile + fileList[ll];
		ofstream outFile(outputFileName, ios::app);

		srand((unsigned)time(NULL));
		read_initial(fileName);//��ȡ����

		for (i = 0; i < 10; i++) {
			Initilize();//��ʼ����Ⱥ
			startTime = clock();//��ʱ��ʼ	
			while ((endTime - startTime) / CLOCKS_PER_SEC < cutting_time)
			{
				sendEmployedBees();

				CalculateProbabilities();

				sendOnlookerBees();

				MemorizeBestSource();

				sendScoutBees();

				MemorizeBestSource();

				endTime = clock();//��ʱ
			}
			outFile << verNum << " " << edgeNum << " " << BestSource.trueFit << " " << bestTime << endl;
		}
	}
	

	

	

}



