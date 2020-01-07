// CREATREG.CPP
// Copyright (c) A.Sobolev 2005
//
#include <iostream.h>
#include <pp.h>
#include <idea.h>
#include <private\regnum.h>

void Usage()
{
	printf("\'-s ClientName OutputFile\' - �� ����� ᤥ���� �਩�� �����. OutputFile - ��� ��室���� 䠩��, ᮤ�ঠ饣� �਩�� �����\n");
	printf("\'-l NumberLicense SerialKeyFile\' - ᮧ���� ����஢���� 䠩� ᮤ�ঠ騩 ���-�� ��業��� (SerialKeyFile - ���� ���)\n");
	printf("\'-r SerialKeyFile MaxLicense ClientName\' - ᮧ���� ����஢���� 䠩� ᮤ�ঠ騩 �਩�� ����, ���-�� ��業���, ��� ������\n");
}

int main(int argc, char ** argv)
{
	PPRegistrInfo reg_info;
	memset(&reg_info, 0, sizeof(reg_info));
	if(argc > 1) {
		if(argv[1][0] == '/' || argv[1][0] == '-') {
			// output help information
			if(argv[1][1] == '?' || toupper(argv[1][1]) == 'H')
				if(argc == 2)
					Usage();
			// create file includes license number
			if(toupper(argv[1][1]) == 'L')
				if(argc == 4) {
					char * inf = 0, file_name[MAXPATH];
					char signature[sizeof(SECURITY_SIGNATURE)];
					FILE * lic_inf_file, * sern_file;

					inf = new char[PPLIC_FILE_SIZE];
					IdeaRandMem(inf, PPLIC_FILE_SIZE);
					STRNSCPY(file_name, argv[3]);
					STRNSCPY(signature, SECURITY_SIGNATURE);
					strtoulong(argv[2], &reg_info.MaxLicense);
					if((sern_file = fopen(file_name, "r")) != NULL) {
						fread(reg_info.SerialKey, sizeof(reg_info.SerialKey), 1, sern_file);
						fclose(sern_file);
						if((lic_inf_file = fopen(PPLIC_FILENAME, "wb")) != NULL) {
							IdeaEncrypt(reg_info.SerialKey, signature, sizeof(SECURITY_SIGNATURE));
							IdeaEncrypt(reg_info.SerialKey, &reg_info.MaxLicense, sizeof(reg_info.MaxLicense));
			   				fwrite(inf, PPLIC_FILE_SIZE, 1, lic_inf_file);
							fseek(lic_inf_file, PPLIC_FILE_OFFS, SEEK_SET);
							fwrite(signature, sizeof(SECURITY_SIGNATURE), 1, lic_inf_file);
							fwrite(&reg_info.MaxLicense, sizeof(reg_info.MaxLicense), 1, lic_inf_file);
							fclose(lic_inf_file);
						}
						else
							printf("\n�訡�� ������ 䠩��: %s", PPLIC_FILENAME);
					}
					else
						printf("\n�訡�� ������ 䠩��: %s", file_name);
					delete [] inf;
				}
				else
					printf("���ࠢ��쭮� ������⢮ ��㬥�⮢\n");
			// create file includes serial (name -> serial number)
			if(toupper(argv[1][1]) == 'S')
				if(argc == 4) {
					FILE * file;
					char file_name[MAXPATH];
					int i;

					STRNSCPY(file_name, argv[3]);
					STRNSCPY(reg_info.ClientName, argv[2]);
					if((file = fopen(file_name, "w")) != NULL) {
						if(CreateSerial(&reg_info) > 0) {
							fwrite(reg_info.SerialKey, sizeof(reg_info.SerialKey), 1, file);
						}
						else
							printf("\n�訡�� ᮧ����� �਩���� ����");
						fclose(file);
					}
					else
						printf("\n�訡�� ������ 䠩��: %s", file_name);
   				}
				else
					printf("���ࠢ��쭮� ������⢮ ��㬥�⮢\n");
			// create registration info file
			if(toupper(argv[1][1]) == 'R')
				if(argc == 5) {
					char * inf = 0;
					char signature[sizeof(SECURITY_SIGNATURE)];
					FILE * reg_inf_file, * sern_file;
					char file_name[MAXPATH];

					inf = new char[PPREG_FILE_SIZE];
					IdeaRandMem(inf, PPREG_FILE_SIZE);
					STRNSCPY(file_name, argv[2]);
					STRNSCPY(signature, SECURITY_SIGNATURE);
					strtoulong(argv[3], &reg_info.MaxLicense);
					STRNSCPY(reg_info.ClientName, argv[4]);
					if((sern_file = fopen(file_name, "r")) != NULL) {
						fread(reg_info.SerialKey, sizeof(reg_info.SerialKey), 1, sern_file);
						fclose(sern_file);
						if((reg_inf_file = fopen(PPREG_FILENAME, "wb")) != NULL) {
							IdeaEncrypt(reg_info.SerialKey, signature, sizeof(SECURITY_SIGNATURE));
							IdeaEncrypt(reg_info.SerialKey, &reg_info.MaxLicense, sizeof(reg_info.MaxLicense));
							IdeaEncrypt(reg_info.SerialKey, reg_info.ClientName, sizeof(reg_info.ClientName));
							IdeaEncrypt(0, reg_info.SerialKey, sizeof(reg_info.SerialKey));
							fwrite(inf, PPREG_FILE_SIZE, 1, reg_inf_file);
							fseek(reg_inf_file, PPREG_FILE_OFFS, SEEK_SET);
							fwrite(signature, sizeof(SECURITY_SIGNATURE), 1, reg_inf_file);
							fwrite(reg_info.SerialKey, sizeof(reg_info.SerialKey), 1, reg_inf_file);
							fwrite(&reg_info.MaxLicense, sizeof(reg_info.MaxLicense), 1, reg_inf_file);
							fwrite(reg_info.ClientName, sizeof(reg_info.ClientName), 1, reg_inf_file);
							fclose(reg_inf_file);
						}
						else
							printf("\n�訡�� ������ 䠩��: %s", PPREG_FILENAME);
					}
					else
						printf("\n�訡�� ������ 䠩��: %s", file_name);
					delete [] inf;
				}
				else
					printf("���ࠢ��쭮� ������⢮ ��㬥�⮢\n");
		}
	}
	else
		Usage();
	return 0;
}
