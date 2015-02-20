/*
 ============================================================================
 Name        : cache-simulator.c
 Author      : Sumanth kumar Bandi
 Version     : 1.0.2
 Copyright   : Copyright 2014, Sumanth kumar Bandi, All rights reserved.
 Description : Cache Simulator
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
//Global Variables------

	unsigned long int ADDRESS = 0;
	char MODE[6];

	unsigned int CACHE_SIZE;
	unsigned int ASSOCIATIVITY;
	unsigned int BLOCK_SIZE;
	char WRITE_POLICY[6];
	unsigned int VICTIM_SIZE;
	unsigned int INDEX;

	long int r_hit,w_hit,v_r_hit,v_w_hit;
	long int r_miss,w_miss,v_r_miss,v_w_miss;
	long int mem_acess;


	unsigned long int TAG,SET;//stores tag and set number for current transaction

	//structure declaration for block
	//This structure is of size 16 bytes
		typedef struct
		{
			unsigned long int tag;
			unsigned long int value;
			unsigned long int age;
			bool valid;
			bool dirty;
		}block_t;

		block_t *l1_cache,*victim;		//pointers for L1 & Victim cache

//Function declarations
	void get_values(char temp[]);
	void get_config(int check, char temp[]);
	block_t * create_cache(block_t *dummy, int size);
	void produce();
	void read();
	void write();
	int search(int type);
	int lru(int type);
	void update_age();
	int search_empty(int type);
	void swap(int i, int j);
	void write_memory(int i);
	void init_block(int i);


//MAIN function-----------------------------------------------------$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
	int main(int argc, char **argv) {

	char temp[40];
	int line=0;

	r_hit=0;
	w_hit=0;
	v_r_hit=0;
	v_w_hit=0;

	r_miss=0;
	w_miss=0;
	v_r_miss=0;
	v_w_miss=0;

	mem_acess=0;
	printf("Input file name is %s\n",argv[1]);

	get_config(argc,argv[2]);  //Getting config to create cache

	if(ASSOCIATIVITY==0)	//If fully associative then, ASSOCIATIVITY = total blocks in cache
			{ASSOCIATIVITY = CACHE_SIZE / BLOCK_SIZE;}
	INDEX=CACHE_SIZE/(BLOCK_SIZE*ASSOCIATIVITY); //calculates no.of blocks in a set

	//Creating L1 & victim cache
	printf(" L1 Cache of size: ");
	l1_cache = create_cache(l1_cache,CACHE_SIZE/BLOCK_SIZE);
	printf(" Victim Cache of size: ");
	victim = create_cache(victim,VICTIM_SIZE*BLOCK_SIZE);

	//Opening file
		FILE *fp;
		fp=fopen(argv[1],"r");
		if(fp==NULL)
			{printf("error");return 0;}

	//Reading from file
		while(!feof(fp))
			{
				line++;

				fgets(temp,40,fp);
				if(!strncmp(temp,"\n",1))	//Comparing for one of the first characters to be \n
				{continue;}
				//printf("\n%d: %s",line,temp);

				get_values(temp);		//stores the address & mode in to ADDRESS & MODE for each line
				produce();				//Produces TAG and SET number for current transaction

				if(strncmp(MODE,"l",1)==0 || strncmp(MODE,"L",1)==0 || strncmp(MODE,"r",1)==0 || strncmp(MODE,"R",1)==0)
					{read();}
				else if(strncmp(MODE,"s",1)==0 || strncmp(MODE,"S",1)==0 || strncmp(MODE,"w",1)==0 || strncmp(MODE,"W",1)==0)
					{write();}
				else
					{printf("Invalid Mode at line %d\n",line);}

				cont:	update_age();

			}
		fclose(fp);

		//Printing the statistics
		printf("\nL1 read hit  :%ld",r_hit);
		printf("\nL1 read miss :%ld",r_miss);
		printf("\nVictim read hit  :%ld",v_r_hit);
		printf("\nVictim read miss :%ld\n",v_r_miss);

		printf("\nL1 write hit  :%ld",w_hit);
		printf("\nL1 write miss :%ld",w_miss);
		printf("\nVictim write hit  :%ld",v_w_hit);
		printf("\nVictim write miss :%ld\n",v_w_miss);

		printf("\nTotal Memory acesses :%ld",mem_acess);

		//Creating output file
						fp=fopen("output.txt","w");
						if(fp==NULL)
							{printf("error");return 0;}

				//Printing the statistics
						fprintf(fp,"\nCACHE_SIZE :%d",CACHE_SIZE);
						fprintf(fp,"\nASSOCIATIVITY :%d",ASSOCIATIVITY);
						fprintf(fp,"\nBLOCK_SIZE :%d",BLOCK_SIZE);
						fprintf(fp,"\nWRITE_POLICY :%s",WRITE_POLICY);
						fprintf(fp,"\nVICTIM_SIZE :%d\n",VICTIM_SIZE);

						fprintf(fp,"\nL1 read hit  :%ld",r_hit);
						fprintf(fp,"\nL1 read miss :%ld",r_miss);
						fprintf(fp,"\nVictim read hit  :%ld",v_r_hit);
						fprintf(fp,"\nVictim read miss :%ld\n",v_r_miss);

						fprintf(fp,"\nL1 write hit  :%ld",w_hit);
						fprintf(fp,"\nL1 write miss :%ld",w_miss);
						fprintf(fp,"\nVictim write hit  :%ld",v_w_hit);
						fprintf(fp,"\nVictim write miss :%ld\n",v_w_miss);

						fprintf(fp,"\nTotal Memory acesses :%ld",mem_acess);
			fclose(fp);

free(l1_cache);
free(victim);
return EXIT_SUCCESS;
}//-MAIN ends------------------------------------------------->$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

//------------------------------------------------------------//
//function name: get_values()
//****Function to interpret the trace file
// containing address in hex format (0x12345678 or 0X12345678)
//------------------------------------------------------------//
void get_values(char temp[])
{
	char *store;
	char *p;

	store=strtok(temp,"  \n");
			do
			{

				if(*store>60)
				{
					strcpy(MODE,store);
						//printf("--%s \t",MODE);
				}
				else if(strncmp(store,"0x",2)==0 || strncmp(store,"0X",2)==0)
				{
					ADDRESS=strtol(store,&p,16);
						//printf("%x -%ld\n",ADDRESS, ADDRESS);
				}


			}
			while(store=strtok(NULL,"  \n"));

return;
}

//------------------------------------------------------------//
//function name: get_config()
//****Function to get the configurations for cache
//.Config file name can be given for configurations as
//command line argument or you can enter the values during run time

//If using .config file please follow the order
// CACHE_SIZE		(in Bytes)
// ASSOCIATIVITY  	(0 for Fully associative,1 for Direct mapped else any integer value )
// BLOCK_SIZE		(in Bytes)
// WRITE_POLICY		(WB for write-back or WT for write-through)
// VICTIM_SIZE		(in BLOCKS)
//------------------------------------------------------------//
void get_config(int check, char temp[])
{

	if (check==3)
	{
		char t[20];
		char *store;
		int line =0;

		printf("Config file name is %s\n",temp);
		FILE *cp;
				cp=fopen(temp,"r");
				if(cp==NULL)
					{printf("error");return;}
		//Reading from file
		while(!feof(cp))
		{
			fgets(t,20,cp);
			if(!strncmp(t,"\n",5))
			{continue;}
			store=strtok(t,"\n");

			switch(line)
			{
			case 0:
				CACHE_SIZE=atoi(store);
				printf("\nCACHE_SIZE :%d",CACHE_SIZE);
				break;
			case 1:
				ASSOCIATIVITY=atoi(store);
				printf("\nASSOCIATIVITY :%d",ASSOCIATIVITY);
				break;
			case 2:
				BLOCK_SIZE=atoi(store);
				printf("\nBLOCK_SIZE :%d",BLOCK_SIZE);
				break;
			case 3:
				strcpy(WRITE_POLICY,store);
				printf("\nWRITE_POLICY :%s",WRITE_POLICY);
				break;
			case 4:
				VICTIM_SIZE=atoi(store);
				printf("\nVICTIM_SIZE :%d\n",VICTIM_SIZE);
				break;
			default:
				printf("\n--CONFIG file error---\n");
				break;
			}//switch ends

		line++;
		}//while ends

	fclose(cp);
	}//if(config file included) ---ends

	else
	{
		printf("No, Config file included so enter configurations\n");

		printf("\nEnter CACHE_SIZE (in Bytes):\t");
			scanf("%d",&CACHE_SIZE);
		printf("\nEnter ASSOCIATIVITY \n(0 for fully associative 1 for direct mapped else any integer value ):\t");
			scanf("%d",&ASSOCIATIVITY);
		printf("\nEnter BLOCK_SIZE	(in Bytes):\t");
			scanf("%d",&BLOCK_SIZE);
		printf("\nEnter WRITE_POLICY \n(WB for write-back or WT for write-through):\t");
			scanf("%s",WRITE_POLICY);
		printf("\nEnter VICTIM_SIZE (in BLOCKS):\t");
			scanf("%d",&VICTIM_SIZE);
	}

return;
}
//End of get_config()

//-------------------------------------------------------------------------------------
//function name: Create_cache()
//creates cache for the given requirements in terms of no of blocks
//-------------------------------------------------------------------------------------
block_t * create_cache(block_t *dummy,int size)
{
	printf("%d Blocks(each %d byes)\n", size, BLOCK_SIZE);
	dummy=(block_t *)malloc(size*sizeof(block_t));

	return dummy;
}
//End of create_cache()


//-------------------------------------------------------------------------------------
//function name: produce()
//
//Uses the ADDRESS provided for current transaction and produces tag & set numbers
//-------------------------------------------------------------------------------------
void produce()
{
	int w=sizeof(ADDRESS)*8;
	int x=log2(CACHE_SIZE);
	int y=log2(BLOCK_SIZE);
	int z=log2(ASSOCIATIVITY);

	SET= ADDRESS<<(w-(y+z));		// (0x12345678 minus TAG) at MSB

		if(ASSOCIATIVITY==1)
			{SET=SET>>(w-z-1);SET=SET>>1;}
		else
			{SET= SET>>(w-z);}				//set number bits only
	TAG= ADDRESS>>(y+z);

	printf("Adress: %lx -Tag: %lx -Set: %lx\n",ADDRESS,TAG,SET);

return;
}
//End of create_cache()


//-------------------------------------------------------------------------------------
//function name: read()
//
//Function to Implement read
//-------------------------------------------------------------------------------------
void read()
{
  int i = search(0);		//searching for the block in L1

	//CODE FOR L1 CACHE HIT----------------------------------
	if(i != -1)			//block found in L1
	{
		r_hit++;		//L1 read hit update
		(l1_cache+( (SET*INDEX)+i ) ) ->age 	= 0;
	}

  //CODE FOR L1 CACHE MISS------------------------------------
	else				//block NOT found in L1
	{
		r_miss++;		//L1 read miss update

	//CODE IF VICTIM CACHE EXISTS----------->>>>>>>>>>>
		if(VICTIM_SIZE != 0)		//victim cache exists
		{
			int j = search(1);	//searching for the block in victim cache

		//CODE FOR VICTIM CACHE HIT------------------------------------
				if(j != -1)		//block found in victim cache
				{
					v_r_hit++;					//Victim read hit update
					int k = search_empty(0);	//searching for empty block in L1

						if(k != -1)		//empty block in L1 found
						{
							swap(k,j);
							(l1_cache+( (SET*INDEX)+ k ) ) ->age 	= 0;
						}
						else			//NO empty block in L1
						{
							k=0;
							k= lru(0);	//searching for LRU block in L1
								swap(k,j);
							(l1_cache+( (SET*INDEX)+ k ) ) ->age 	= 0;
						}

				}

		//CODE FOR VICTIM CACHE MISS------------------------------------
				else
				{
					v_r_miss++;					//Victim read miss update
					int k = search_empty(0);	//searching for empty block in L1
						if(k != -1)		//empty block in L1 found
						{
							init_block(k);	//update the L1 cache block with new values
						}
						else			//NO empty block in L1
						{
							k=0;
							k= lru(0);				//searching for LRU block in L1
							int m =search_empty(1);	//searching for empty block in Victim
								if(m != -1)		//empty block in Victim cache found
								{
									swap(k,m);
									init_block(k);
								}
								else			//NO empty block in Victim cache
								{
									m = 0;
									m = lru(1);		//searching for LRU block in victim
									swap(k,m);
									init_block(k);	//update the L1 cache block with new values
								}
						}
				}
		}

	//CODE IF VICTIM CACHE DOES NOT EXIST----------->>>>>>>>>>>
		else				//VICTIM_SIZE = 0
		{
			int k = search_empty(0);	//searching for empty block in L1
				if(k != -1)		//empty block in L1 found
				{
					init_block(k);	//update the L1 cache block with new values
				}
				else			//NO empty block in L1
				{
					k=0;
					k=lru(0);		//searching for LRU block in L1
					init_block(k);	//update the L1 cache block with new values
				}
		}


	}


return;
}
//read() ends--


//-------------------------------------------------------------------------------------
//function name: write()
//
//Function to Implement write
//-------------------------------------------------------------------------------------
void write()
{
	if(  ((strncmp(WRITE_POLICY,"WB",2)==0) || (strncmp(WRITE_POLICY,"wb",2)==0)) )
	{
		int i = search(0);//searching for the block in L1

		//CODE FOR L1 CACHE HIT----------------------------------
		if(i != -1)			//block found in L1
		{
			w_hit++;		//L1 write hit update
			(l1_cache+( (SET*INDEX)+i ) ) ->age 	= 0;
			(l1_cache+( (SET*INDEX)+i ) ) ->dirty 	= 1;
		}

		//CODE FOR L1 CACHE MISS------------------------------------
			else				//block NOT found in L1
			{
				w_miss++;		//L1 write miss update

			//CODE IF VICTIM CACHE EXISTS----------->>>>>>>>>>>
				if(VICTIM_SIZE != 0)		//victim cache exists
				{
					int j = search(1);	//searching for the block in victim cache

				//CODE FOR VICTIM CACHE HIT------------------------------------
						if(j != -1)		//block found in victim cache
						{
							v_w_hit++;					//Victim write hit update
							int k = search_empty(0);	//searching for empty block in L1

								if(k != -1)		//empty block in L1 found
								{
									swap(k,j);
									(l1_cache+( (SET*INDEX)+ k ) ) ->age 	= 0;
									(l1_cache+( (SET*INDEX)+ k ) ) ->dirty 	= 1;
								}
								else			//NO empty block in L1
								{
									k=0;
									k= lru(0);	//searching for LRU block in L1
										if((l1_cache+( (SET*INDEX)+ k ) ) ->dirty 	== 1)
										{write_memory(k);}
										swap(k,j);
									(l1_cache+( (SET*INDEX)+ k ) ) ->age 	= 0;
									(l1_cache+( (SET*INDEX)+ k ) ) ->dirty 	= 1;
								}

						}

				//CODE FOR VICTIM CACHE MISS------------------------------------
						else
						{
							v_w_miss++;					//Victim write miss update
							int k = search_empty(0);	//searching for empty block in L1

								if(k != -1)		//empty block in L1 found
								{
									init_block(k);	//update the L1 cache block with new values
								}
								else			//NO empty block in L1
								{
									k=0;
									k= lru(0);				//searching for LRU block in L1
										if((l1_cache+( (SET*INDEX)+ k ) ) ->dirty 	== 1)
											{write_memory(k);}
									int m =search_empty(1);	//searching for empty block in Victim
										if(m != -1)		//empty block in Victim cache found
										{
											swap(k,m);
											init_block(k);
										}
										else			//NO empty block in Victim cache
										{
											m = 0;
											m = lru(1);		//searching for LRU block in victim
											swap(k,m);
											init_block(k);	//update the L1 cache block with new values
										}
								}
							}
				}

				//CODE IF VICTIM CACHE DOES NOT EXIST----------->>>>>>>>>>>
						else				//VICTIM_SIZE = 0
						{
							int k = search_empty(0);	//searching for empty block in L1
								if(k != -1)		//empty block in L1 found
								{
									init_block(k);	//update the L1 cache block with new values
								}
								else			//NO empty block in L1
								{
									k=0;
									k=lru(0);		//searching for LRU block in L1
										if((l1_cache+( (SET*INDEX)+ k ) ) ->dirty 	== 1)
											{write_memory(k);}
									init_block(k);	//update the L1 cache block with new values
								}
						}
			}
	} //WB policy -- write allocate ends

	//WT policy -- No write allocate
	else
	{
		int i = search(0);
		//CODE FOR L1 CACHE HIT----------------------------------
			if(i != -1)			//block found in L1
			{
				w_hit++;		//L1 write hit update
				(l1_cache+( (SET*INDEX)+i ) ) ->age 	= 0;
				write_memory(0);
			}
			//CODE FOR L1 CACHE MISS------------------------------------
						else				//block NOT found in L1
						{
							w_miss++;		//L1 write miss update

						//CODE IF VICTIM CACHE EXISTS----------->>>>>>>>>>>
							if(VICTIM_SIZE != 0)		//victim cache exists
							{
								int j = search(1);	//searching for the block in victim cache
								write_memory(0);

							//CODE FOR VICTIM CACHE HIT------------------------------------
									if(j != -1)		//block found in victim cache
									{
										v_w_hit++;					//Victim write hit update
										int k = search_empty(0);	//searching for empty block in L1

											if(k != -1)		//empty block in L1 found
											{
												swap(k,j);
												(l1_cache+( (SET*INDEX)+ k ) ) ->age 	= 0;
											}
											else			//NO empty block in L1
											{
												k=0;
												k= lru(0);	//searching for LRU block in L1
												swap(k,j);
												(l1_cache+( (SET*INDEX)+ k ) ) ->age 	= 0;
											}
									}

							//CODE FOR VICTIM CACHE MISS------------------------------------
									else
									{
										v_w_miss++;					//Victim write miss update
									}
							}

						//CODE IF VICTIM CACHE DOES NOT EXIST----------->>>>>>>>>>>
						else				//VICTIM_SIZE = 0
						{write_memory(0);}

				}//L1 miss code ends

	}//WT policy ends


return;
}
//write() ends--


//-------------------------------------------------------------------------------------
//function name: search()
//
//To search for particular tag value in a cache
//arguments type: 0 for L1 cache search, 1 for victim search

//returns: -1 not found else returns offset from the starting of the set
//-------------------------------------------------------------------------------------
int search(int type)
{
	int i=0;

	if(type == 0)		//to search inside L1 cache
	{
		for(i=0;i<INDEX;i++)
		{
			if( (l1_cache+(SET*INDEX)+i)->tag == TAG && (l1_cache+(SET*INDEX)+i)->valid == 1 )
				{return i;}
			else
				{continue;}
		}//for end
	}//if ends

	else				//to search inside victim cache
	{
			for(i=0;i< (VICTIM_SIZE) ;i++)
			{
				if( (victim +i)->tag == TAG && (victim +i)->valid ==1 )
						{return i;}
				else
						{continue;}
			}//for end
	}//else ends

return -1;
}

//-------------------------------------------------------------------------------------
//function name: lru()
//
//In a given instance it finds the least used block in a set

//returns offset from the beginning of the set
//returns -1 if fails to find
//-------------------------------------------------------------------------------------
int lru(int type)
{
	int i=0;
	int k=0;
	unsigned long int j = 0;
		if(type == 0)		//to search inside L1 cache
		{
			for(i=0;i<INDEX;i++)
			{
				if( ((l1_cache+(SET*INDEX)+i)->age) >= j && (l1_cache+(SET*INDEX)+i)->valid == 1 )
					{
					j = (l1_cache+(SET*INDEX)+i)->age;//update new higher age
					k = i;							//Note down the offset
					}
				else
					{continue;}
			}//for end

			return k;
		}//if ends

		else				//to search inside victim cache
		{
				for(i=0;i< (VICTIM_SIZE) ;i++)
				{
					if( (victim +i)->age >= j && (victim +i)->valid ==1 )
							{
							j = (victim +i)->age ;		//update new higher age
							k = i;						//Note down the offset
							}
					else
							{continue;}
				}//for end

			return k;
		}//else ends

return -1;
}

//-------------------------------------------------------------------------------------
//function name: update_age()
//
//Updates the age of all VALID blocks of L1 & victim cache
//-------------------------------------------------------------------------------------
void update_age()
{
	int i=0;

	for(i=0; i < (CACHE_SIZE/BLOCK_SIZE) ;i++)		//updating age for all valid l1 cache blocks
	{
		if( (l1_cache+i)->valid == 1)
			{((l1_cache+i)->age) = ((l1_cache+i)->age)+1;}
		else
			{continue;}

	}

	for(i=0; i < (VICTIM_SIZE) ;i++)		//updating age for all valid Victim cache blocks
		{
			if( (victim+i)->valid == 1)
				{((victim+i)->age) = ((victim+i)->age)+1;}
			else
				{continue;}

		}

return;
}

//-------------------------------------------------------------------------------------
//function name: search_empty()
//
//To search for empty blocks in a cache
//arguments type: 0 for L1 cache search, 1 for victim search

//returns: -1 not found any empty blocks else returns offset from the starting of the set
//-------------------------------------------------------------------------------------
int search_empty(int type)
{
	int i=0;

		if(type == 0)		//to search inside L1 cache
		{
			for(i=0;i<INDEX;i++)
			{
				if( (l1_cache+(SET*INDEX)+i)->valid == 0 )
					{return i;}
				else
					{continue;}
			}//for end
		}//if ends

		else				//to search inside victim cache
		{
				for(i=0;i< (VICTIM_SIZE) ;i++)
				{
					if( (victim +i)->valid == 0 )
							{return i;}
					else
							{continue;}
				}//for end
		}//else ends


return -1;

}
//search_empty() ends


//-------------------------------------------------------------------------------------
//function name: swap()
//
//swaps block between l1 cache & victim cache
//argument 1: offset number of block in a set of l1 cache
//argument 2: offset number of block in victim cache
//-------------------------------------------------------------------------------------
void swap(int i, int j)
{
unsigned long int k;
bool v;

	//checking for dirty bit to be set and write policy to be write-back
	if( (l1_cache+((SET*INDEX)+i))->dirty==1 && ((strncmp(WRITE_POLICY,"WB",2)==0) || (strncmp(WRITE_POLICY,"wb",2)==0)) )
	{
		//Function to write in to memory
		write_memory(i);
	}

	//swap tag
		k 	= 	(victim+j)->tag;
		(victim+j)->tag = (l1_cache+((SET*INDEX)+i))->tag;
		(l1_cache+((SET*INDEX)+i))->tag = k;

	//swap age
		k 	= 	(victim+j)->age;
		(victim+j)->age = (l1_cache+((SET*INDEX)+i))->age;
		(l1_cache+((SET*INDEX)+i))->age = k;

	//swap validity
		v 	= 	(victim+j)->valid;
		(victim+j)->valid = (l1_cache+((SET*INDEX)+i))->valid;
		(l1_cache+((SET*INDEX)+i))->valid = v;


return;
}
//swap() ends

//-------------------------------------------------------------------------------------
//function name: write_memory()
//
//write backs the value of a block back to the memory and resets the dirty bit
//-------------------------------------------------------------------------------------
void write_memory(int i)
{
	//
	//write to memory
	//
	mem_acess++;

	if( (l1_cache+((SET*INDEX)+i))->dirty==1 && ((strncmp(WRITE_POLICY,"WB",2)==0) || (strncmp(WRITE_POLICY,"wb",2)==0)) )
	{
	(l1_cache+( (SET*INDEX)+i ) )->dirty = 0;
	}
return;
}

//-------------------------------------------------------------------------------------
//function name: init_block()
//
//writes values into block directly
//argument: offset of the block in a set
//-------------------------------------------------------------------------------------
void init_block(int i)
{
	mem_acess++;

	(l1_cache+( (SET*INDEX)+i ) ) ->tag 	= TAG;
	(l1_cache+( (SET*INDEX)+i ) ) ->valid 	= 1;
	(l1_cache+( (SET*INDEX)+i ) ) ->age 	= 0;

	if(  ((strncmp(WRITE_POLICY,"WB",2)==0) || (strncmp(WRITE_POLICY,"wb",2)==0)) )
	{
		if(strncmp(MODE,"l",1)==0 || strncmp(MODE,"L",1)==0 || strncmp(MODE,"r",1)==0 || strncmp(MODE,"R",1)==0)
		{ (l1_cache+( (SET*INDEX)+i ) ) ->dirty 	= 0; }
		else
		{ (l1_cache+( (SET*INDEX)+i ) ) ->dirty 	= 1; }
	}

return;
}
