// MLCP_Study_Restart.cpp : �������̨Ӧ�ó������ڵ㡣
//ѧϰ����,����ʵ��汾

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

//�ð汾������swap������tt�̶�15��alpha0.01��omega1000����exp���omega���ˣ�tt����
using namespace std;
#define MAX_VAL 999999

clock_t startTime, endTime;//��¼����ʱ��
double bestTime;//���ʱ��
double cutting_time = 1800;//��ֹ����
string inFile = "./data/homer.col";
string outFile = "./experiment_tt10_swap_exp4.txt";

//��Ҫ���ڵĲ���
int tt = 10;//���ɳ���
float pertubationDepth = 0.01;//�Ŷ�ǿ��
int restartThreshold = 100;//����restart����ֵ
int pertubationThreshold = 500;//�����Ŷ�����ֵ
int noimprove = 0;//����δ���µĴ���
int pertubationTime = 0;//С���Ŷ��Ĵ���
float alpha = 0.1;
float beta = 0.2;//�������ֵ��Ҫ��ʵ��
float gamma1 = 0.3;


int Sc;
int Scb;
int Sb;

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
int random_int(int);
void read_initial(string);
void random_initial();//���������ʼ��
void compute_obj();//����Ŀ��ֵ
void FLIP(int n);//���ɻ�flt�ӵ�ǰgate�ƶ���gt
void show_result();
void local_search();
void judge_best();
void pertubation();//�Ŷ����Ŷ����ǵ�ǰ��
void restart();
void updateP();

//�������һ�� 0-(n-1) ������
int random_int(int n)
{
	return rand() % n;
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

//���������ʼ��
void random_initial()
{
	int i, j, k, m;
	colorlen[0] = 0;
	colorlen[1] = 0;
	bian[0] = 0;
	bian[1] = 0;
	for (i = 0; i < verNum; i++)
	{
		tabu[i] = 0;
		conect[i] = 0;
		p[i][0] = 0.5;
		p[i][1] = 0.5;
	}
	for (i = 0; i < verNum; i++)
	{
		k = random_int(2);
		color[k][colorlen[k]] = i;
		v2color[i] = k;//�����ɫ
		initialColor[i] = k;
		currentBestColor[i] = k;
		bestColor[i] = k;
		v2idx[i] = colorlen[k];//�ǵڼ����������ɫ��
		colorlen[k]++;
	}
	for (i = 0; i < verNum; i++)
		for (j = 0; j < verNum; j++)
			if (edge[i][j])
				if (v2color[i] == v2color[j])//�������Լ���ɫ��ͬ���б������ģ��ͼ���1
					conect[i]++;
	for (i = 0; i < verNum; i++)
	{
		k = v2color[i];
		bian[k] += conect[i];//�ۼӵĻ���������Σ���Ҫ����2
	}
	bian[0] /= 2;
	bian[1] /= 2;
	Sc = min(bian[0], bian[1]);
	Scb = Sc;
	Sb = Sc;
}

//�ֲ�����
void local_search()
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
	//cout << pro << endl;
	if (rand() % 100 < pro) {
		//FLIP
		for (i = 0; i < verNum; i++)
		{
			if (tabu[i] > iter)//��������ˣ���ֱ������
				continue;

			k = bian[1 - v2color[i]] + adjlen[i] - conect[i];//�������һ�������ټ��㷽����������֮���������еĵ㣬��ȥ���Լ���ɫԭ����ͬ�ĵ㣬ʣ�µľ������Լ���ɫԭ����ͬ�ĵ�
			m = bian[v2color[i]] - conect[i];//�Ƴ�ȥ����һ������ȥ���Լ���ɫԭ����ͬ�ĵ�
			delta = min(k, m) - Sc;//����delta
			if (delta > temp_delta)//�������flip����
			{
				temp_delta = delta;
				mark = i;
			}
			else if (delta == temp_delta) {//����ڶ�������
				if (k + m > temp_delta_2) {
					temp_delta_2 = k + m;
					mark = i;
				}
			}
		}
		if (mark == -1)
		{
			cout << "mark==-1" << endl;
			getchar();
		}
		else {
			tabu[mark] = iter + tt;
			FLIP(mark);//�����ݽṹ���и���
			if (Sc > Sb) {
				pro++;
			}
		}
	}
	else {
		//SWAPmove
		for (i = 0; i < colorlen[0]; i++)
		{
			org_0 = color[0][i];//0ɫ�ĵ�
			if (tabu[org_0] > iter)
				continue;
			for (j = 0; j < colorlen[1]; j++)
			{
				org_1 = color[1][j];//1ɫ�ĵ�
				if (tabu[org_1] > iter)//��Υ����������
					continue;
				k = bian[1] + adjlen[org_0] - conect[org_0] - conect[org_1] - edge[org_0][org_1];//����֮����Ҫ��ע������֮���Ƿ��б�����������б���������Ҫ�����������߼������൱���Ƕ���
				m = bian[0] + adjlen[org_1] - conect[org_1] - conect[org_0] - edge[org_1][org_0];
				delta = min(k, m) - Sc;
				if (delta > swap_delta)//�������swap����
				{
					swap_delta = delta;
					swap_0 = org_0;
					swap_1 = org_1;
				}
				else if (delta == swap_delta) {
					if (k + m > swap_delta_2) {
						swap_delta_2 = k + m;
						swap_0 = org_0;
						swap_1 = org_1;
					}
				}
			}
		}

		if (swap_0 == -1 || swap_1 == -1) {
			//cout << swap_0 << " " << swap_1 << endl;
		}
		else {
			tabu[swap_0] = iter + tt;
			tabu[swap_1] = iter + tt;
			FLIP(swap_0);//�����ݽṹ���и��£��൱�����ƹ�ȥ�ˣ����ƹ���
			FLIP(swap_1);
		}
		if (Sc > Sb) {
			pro--;
		}
	}

	//cout << "mark: " << mark+1 << " temp_delta: " << temp_delta << endl;
	judge_best();//�������Ž�

}


//��ת����
void FLIP(int n)
{
	int org_color;
	int org_idx;
	int i, j, k, m;
	org_color = v2color[n];//�ŵ���ɫ,��Ե�ǰ��
	org_idx = v2idx[n];//�ڼ���

	color[1 - org_color][colorlen[1 - org_color]] = n;//��ɫ
	v2color[n] = 1 - org_color;
	v2idx[n] = colorlen[1 - org_color];
	colorlen[1 - org_color]++;//��ɫ������1
	bian[org_color] -= conect[n];
	conect[n] = adjlen[n] - conect[n];//����
	bian[1 - org_color] += conect[n];//�ı�����bian

	colorlen[org_color]--;//��ɫ������1
	m = color[org_color][colorlen[org_color]];//���һ������ȥ
	color[org_color][org_idx] = m;
	v2idx[m] = org_idx;

	//����conect
	for (i = 0; i < adjlen[n]; i++)//�͵������ӵĵ��б仯
	{
		k = adj[n][i];
		m = v2color[k];
		if (m == org_color)//���ϵ���ɫһ��
			conect[k]--;
		else//���ϵ���ɫ��һ��
			conect[k]++;
	}
	Sc = min(bian[0], bian[1]);//����Sc
}

void judge_best()
{
	int i;
	if (Sc > Scb)
	{
		Scb = Sc;
		noimprove = 0;
		for (i = 0; i < verNum; i++) {
			currentBestColor[i] = v2color[i];
		}
		//compute_obj();
		if (Sc > Sb) {
			endTime = clock();//��ʱ
			bestTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
			Sb = Sc;
			for (i = 0; i < verNum; i++) {
				bestColor[i] = v2color[i];
			}
			//cout << "iter: " << iter << " Sc: " << Sc << " time: " << bestTime << "s" << endl;
		}
	}
	else
	{
		noimprove++;
		if (noimprove > pertubationThreshold)
		{
			pro = 50;
			noimprove = 0;
			pertubationTime++;
			if (pertubationTime < restartThreshold) {
				pertubation();//С���Ŷ�
			}
			else {
				//cout << "restart" << endl;
				pertubationTime = 0;
				restart();//����
			}
		}
	}
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

void updateP() {
	int i, j, len_0_0, len_0_1;
	int len = 0;
	len_0_0 = len_0_1 = 0;
	for (i = 0; i < verNum; i++) {
		lastColor[i] = currentBestColor[i];
	}
	//�޸��Գ���
	for (i = 0; i < verNum; i++) {
		if (initialColor[i] == 0 && lastColor[i] == 0) {
			len_0_0++;
		}
		else if (initialColor[i] == 0 && lastColor[i] == 1) {
			len_0_1++;
		}
	}
	if (len_0_1 > len_0_0) {
		for (i = 0; i < verNum; i++) {
			if (lastColor[i] == 0) {
				lastColor[i] = 1;
			}
			else {
				lastColor[i] = 0;
			}
		}
	}

	//���жԱ�
	for (i = 1; i < verNum; i++) {
		if (lastColor[i] == initialColor[i]) {//ֱ�Ӽ���
			p[i][lastColor[i]] = alpha + (1 - alpha)*p[i][lastColor[i]];
			p[i][1 - lastColor[i]] = (1 - alpha)*p[i][1 - lastColor[i]];
		}
		else {//�ͷ��ϵģ������µ�
			p[i][lastColor[i]] = gamma1 + (1 - gamma1)*beta + (1 - gamma1)*(1 - beta)*p[i][lastColor[i]];
			p[i][initialColor[i]] = (1 - gamma1)*(1 - beta)*p[i][initialColor[i]];
		}
	}

	/*for (i = 1; i < verNum; i++) {
	cout << i << " " << p[i][0] << " " << p[i][1] << endl;
	}*/

}

void restart()//����
{
	int i, j, k, m;
	int l;
	colorlen[0] = 0;
	colorlen[1] = 0;
	bian[0] = 0;
	bian[1] = 0;
	for (i = 0; i < verNum; i++)
	{
		tabu[i] = 0;
		conect[i] = 0;
	}
	//��һ������ɫ��Ҫ�Ľ�
	updateP();
	for (i = 0; i < verNum; i++)
	{
		//���̶�
		l = p[i][0] / (p[i][0] + p[i][1]) * 100;
		//cout << "l = " << l << endl;
		if (random_int(101) < l)
			k = 0;
		else if (random_int(101) == l)
			k = random_int(2);
		else
			k = 1;

		//k = random_int(2);
		color[k][colorlen[k]] = i;
		v2color[i] = k;//�����ɫ
		initialColor[i] = k;
		v2idx[i] = colorlen[k];//�ǵڼ����������ɫ��
		colorlen[k]++;
	}
	//����
	for (i = 0; i < verNum; i++)
		for (j = 0; j < verNum; j++)
			if (edge[i][j])
				if (v2color[i] == v2color[j])//�������Լ���ɫ��ͬ���б������ģ��ͼ���1
					conect[i]++;
	for (i = 0; i < verNum; i++)
	{
		k = v2color[i];
		bian[k] += conect[i];//�ۼӵĻ���������Σ���Ҫ����2
	}
	bian[0] /= 2;
	bian[1] /= 2;
	Sc = min(bian[0], bian[1]);
	Scb = Sc;
}

void pertubation()//�Ŷ�
{
	int i, j, k;
	k = max(int(pertubationDepth * verNum), 1);
	//cout <<"iter: "<<iter<< " �Ŷ�ǰ��sc: " << Sc << endl;
	for (i = 0; i < k; i++)
	{
		j = random_int(verNum);//��������ƶ�
		FLIP(j);
		tabu[j] = 0;
	}
	//cout << "iter: " << iter<< "�Ŷ���sc: " << Sc << endl;
}

//������
int main(int argc, char** argv)
{
	int i, j;
	string inputFile = inFile;
	string outputFile = outFile;
	srand((unsigned)time(NULL));
	read_initial(inputFile);//��ȡ����
	ofstream outFile(outputFile, ios::app);

	for (i = 0; i < 10; i++) {
		startTime = clock();//��ʱ��ʼ	
		endTime = startTime;
		random_initial();//�����ʼ����������ķ�ʽ
		iter = 0;//��������
		while ((endTime - startTime) / CLOCKS_PER_SEC < cutting_time)
		{
			//show_result();
			iter++;
			local_search();
			endTime = clock();//��ʱ
		}
		outFile << beta << " " << verNum << " " << edgeNum << " " << Sb << " " << bestTime << endl;
	}
}



