// MLCP_ABC.cpp : 定义控制台应用程序的入口点。
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

//该版本加入了swap操作，tt固定15，alpha0.01，omega1000，与exp相比omega变了，tt变了
using namespace std;
#define MAX_VAL 999999

clock_t startTime, endTime;//记录运行时间
double bestTime;//最佳时间
double cutting_time = 180;//终止条件
//string inFile = "C:/Users/18367/Documents/Visual Studio 2015/Projects/MLCP/Debug/data/le450_15c.col";

string inFile = "C:/Users/18367/Documents/Visual Studio 2015/Projects/MLCP/Debug/data/";
string outFile = "F:/科研文献/论文1/ABC/180/";

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
//主要调节的参数
int tt = 10;//禁忌长度
float pertubationDepth = 0.01;//扰动强度
int restartThreshold = 10;//触发restart的阈值
int pertubationThreshold = 500;//触发扰动的阈值
int noimprove = 0;//连续未更新的次数
int pertubationTime = 0;//小型扰动的次数
float alpha = 0.1;
float beta = 0.2;//具体多少值需要做实验
float gamma1 = 0.3;
const int NP = 40;//种群的规模，采蜜蜂+观察蜂
const int FoodNumber = NP / 2;//食物的数量，为采蜜蜂的数量
const int maxCycle = 10000;//停止条件
const int limit = 20;//限度，超过这个限度没有更新采蜜蜂变成侦查蜂


int edgeNum, verNum;
int** edge;//两点间是否有边
int** adj;//
int* adjlen;//

			//主要数据结构
int** color;
int* v2color;//点的当前着色
int* currentBestColor;//当前最优着色
int* bestColor;//全局最优着色
int* initialColor;//当前着色
int* lastColor;//最优着色
			   //针对当前解的数据结构
int* colorlen;
int* v2idx;
int* conect;//与所在颜色的边
int* bian;//两个颜色中边的数量
		  //restart数据结构
float** p;


//局部搜索
int* tabu;//禁忌表
int iter = 0;//迭代次数
int pro = 50;//选择邻域的概率

//函数定义


int Sblist[10];
double timelist[10];

struct BeeGroup
{
	int code[1000];//函数的维数0-1
	int bian[2];
	int conect[1000];
	int trueFit;//记录目标函数值
	double rfitness;//相对适应值比例
	int trail;//表示实验的次数，用于与limit作比较
}Bee[FoodNumber];

BeeGroup NectarSource[FoodNumber];//蜜源，注意：一切的修改都是针对蜜源而言的
BeeGroup EmployedBee[FoodNumber];//采蜜蜂
BeeGroup OnLooker[FoodNumber];//观察蜂
BeeGroup BestSource;//记录最好蜜源

//随机生成一个 0-(n-1) 的整数
int random_int(int n)
{
	return rand() % n;
}

/*****函数的实现****/
double random(double start, double end)//随机产生区间内的随机数
{
	return start + (end - start)*rand() / (RAND_MAX + 1.0);
}

//读取数据
void read_initial(string inFile)
{
	int i, j, k;
	ifstream FIC;
	FIC.open(inFile);//读取数据
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
	edge = new int*[verNum];//记录哪两个点相连
	adj = new int*[verNum];//记录点与哪些点相连
	color = new int*[2];//记录每种颜色都有哪些点
	p = new float*[verNum];
	adjlen = new int[verNum];//点与哪些点相连的长度
	tabu = new int[verNum];
	v2color = new int[verNum];
	currentBestColor = new int[verNum];
	bestColor = new int[verNum];
	initialColor = new int[verNum];
	lastColor = new int[verNum];
	v2idx = new int[verNum];
	conect = new int[verNum];
	colorlen = new int[2];//两种颜色着色的点数计数
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
		edge[a][b] = edge[b][a] = 1;//如果二者有边相连

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

	FIC.close();//关闭数据
}

double calculationTruefit(BeeGroup bee)//计算真实的函数值
{
	int i, j, k;
	bee.bian[0] = 0;
	bee.bian[1] = 0;
	for (i = 0; i < verNum; i++)
	{
		bee.conect[i] = 0;
	}
	/******测试函数1******/
	for (i = 0; i < verNum; i++)
		for (j = 0; j < verNum; j++)
			if (edge[i][j])
				if (bee.code[i] == bee.code[j])//遇到与自己着色相同且有边相连的，就计数1
					bee.conect[i]++;
	for (i = 0; i < verNum; i++)
	{
		k = bee.code[i];
		bee.bian[k] += bee.conect[i];//累加的话会计算两次，需要除以2
	}
	bee.bian[0] /= 2;
	bee.bian[1] /= 2;
	int Sc = min(bee.bian[0], bee.bian[1]);

	return Sc;
}

//随机构建初始解
void random_initial()
{
	int i, j, k, m;
	for (i = 0; i < verNum; i++)
	{
		k = random_int(2);
		v2color[i] = k;//点的着色
	}
}

void Initilize()
{
	int i, j;
	for (i = 0; i<FoodNumber; i++)//对于FoodNumber个种群
	{
		random_initial();
		for (j = 0; j<verNum; j++)//
		{
			NectarSource[i].code[j] = v2color[j];
			EmployedBee[i].code[j] = NectarSource[i].code[j];//采蜜蜂
			OnLooker[i].code[j] = NectarSource[i].code[j];//观察蜂
			BestSource.code[j] = NectarSource[0].code[j];//最佳蜜源
		}
		/****蜜源的初始化*****/
		NectarSource[i].trueFit = calculationTruefit(NectarSource[i]);
		NectarSource[i].rfitness = 0;
		NectarSource[i].trail = 0;
		/****采蜜蜂的初始化*****/
		EmployedBee[i].trueFit = NectarSource[i].trueFit;
		EmployedBee[i].rfitness = NectarSource[i].rfitness;
		EmployedBee[i].trail = NectarSource[i].trail;
		/****观察蜂的初始化****/
		OnLooker[i].trueFit = NectarSource[i].trueFit;
		OnLooker[i].rfitness = NectarSource[i].rfitness;
		OnLooker[i].trail = NectarSource[i].trail;
	}
	/*****最优蜜源的初始化*****/
	BestSource.trueFit = NectarSource[0].trueFit;
	BestSource.rfitness = NectarSource[0].rfitness;
	BestSource.trail = NectarSource[0].trail;
}

//翻转操作
BeeGroup FLIP(BeeGroup bee, int n)
{
	int org_color;
	int org_idx;
	int i, j, k, m;
	org_color = bee.code[n];//着的颜色,针对当前解
	bee.code[n] = 1 - org_color;
	bee.bian[org_color] -= bee.conect[n];
	bee.conect[n] = adjlen[n] - bee.conect[n];//补集
	bee.bian[1 - org_color] += bee.conect[n];//改变两个bian

	//更新conect
	for (i = 0; i < adjlen[n]; i++)//和点有连接的点有变化
	{
		k = adj[n][i];
		m = bee.code[k];
		if (m == org_color)//和老的颜色一样
			bee.conect[k]--;
		else//和老的颜色不一样
			bee.conect[k]++;
	}
	return bee;
}


//局部搜索
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
		k = bee.bian[1 - bee.code[i]] + adjlen[i] - bee.conect[i];//移入的那一方，快速计算方法，加上与之相连的所有的点，减去与自己颜色原本相同的点，剩下的就是与自己颜色原本不同的点
		m = bee.bian[bee.code[i]] - bee.conect[i];//移出去的那一方，减去与自己颜色原本相同的点
		delta = min(k, m) - Sc;//计算delta
		if (delta > temp_delta)//保留最佳flip操作
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
		bee = FLIP(bee, mark);//对数据结构进行更新
	}

	return bee;
}


void show_result()
{
	int i, j, k, m;

	cout << "颜色0[" << colorlen[0] << "]bian0[" << bian[0] << "]: ";
	for (i = 0; i < colorlen[0]; i++)
	{
		cout << color[0][i] + 1 << "[" << conect[color[0][i]] << "] ";
	}
	cout << endl << "颜色1[" << colorlen[1] << "]bian1[" << bian[1] << "]: ";
	for (i = 0; i < colorlen[1]; i++)
	{
		cout << color[1][i] + 1 << "[" << conect[color[1][i]] << "] ";
	}
	cout << endl;

}


void sendEmployedBees()//修改采蜜蜂的函数
{
	int i, j, k;
	double Rij;//[-1,1]之间的随机数
	for (i = 0; i<FoodNumber; i++)
	{
		for (j = 0; j<verNum; j++)
		{
			EmployedBee[i].code[j] = NectarSource[i].code[j];
		}

		EmployedBee[i].trueFit = calculationTruefit(EmployedBee[i]);

		EmployedBee[i] = local_search(EmployedBee[i]);

		/*******采蜜蜂去更新信息*******/
		EmployedBee[i].trueFit = calculationTruefit(EmployedBee[i]);

		//cout << i + 1 << " " << EmployedBee[i].trueFit << endl;
		/******贪婪选择策略*******/
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
	double R_choosed;//被选中的概率
	i = 0;
	t = 0;
	while (t < FoodNumber)
	{
		R_choosed = random(0, 1);
		//cout << R_choosed << " " << NectarSource[i].rfitness << endl;
		if (R_choosed < NectarSource[i].rfitness)//根据被选择的概率选择
		{
			t++;
			/******选取不等于i的k********/
			for (j = 0; j<verNum; j++)
			{
				OnLooker[i].code[j] = NectarSource[i].code[j];
			}

			/****更新****/

			OnLooker[i].trueFit = calculationTruefit(OnLooker[i]);

			//cout << i + 1 << " " << OnLooker[i].trueFit << endl;

			OnLooker[i] = local_search(OnLooker[i]);

			OnLooker[i].trueFit = calculationTruefit(OnLooker[i]);

			/****贪婪选择策略******/
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

void CalculateProbabilities()//计算轮盘赌的选择概率
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

void MemorizeBestSource()//保存最优的蜜源
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
			endTime = clock();//计时
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
			k = bee.bian[1 - bee.code[i]] + adjlen[i] - bee.conect[i];//移入的那一方，快速计算方法，加上与之相连的所有的点，减去与自己颜色原本相同的点，剩下的就是与自己颜色原本不同的点
			m = bee.bian[bee.code[i]] - bee.conect[i];//移出去的那一方，减去与自己颜色原本相同的点
			delta = min(k, m) - Sc;//计算delta
			if (delta > temp_delta)//保留最佳flip操作
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
		bee = FLIP(bee, mark);//对数据结构进行更新
	}

	return bee;
}

/*******只有一只侦查蜂**********/
void sendScoutBees()//判断是否有侦查蜂的出现，有则重新生成蜜源
{
	int i, j;
	for (i = 1; i<FoodNumber; i++)
	{
		if (NectarSource[i].trail >= limit)
		{
			/*******重新初始化*********/
			NectarSource[i] = local_search_Scout(NectarSource[i]);
			NectarSource[i].trail = 0;
			NectarSource[i].trueFit = calculationTruefit(NectarSource[i]);
		}
	}
}

//主函数
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
		read_initial(fileName);//读取数据

		for (i = 0; i < 10; i++) {
			Initilize();//初始化蜂群
			startTime = clock();//计时开始	
			while ((endTime - startTime) / CLOCKS_PER_SEC < cutting_time)
			{
				sendEmployedBees();

				CalculateProbabilities();

				sendOnlookerBees();

				MemorizeBestSource();

				sendScoutBees();

				MemorizeBestSource();

				endTime = clock();//计时
			}
			outFile << verNum << " " << edgeNum << " " << BestSource.trueFit << " " << bestTime << endl;
		}
	}
	

	

	

}



