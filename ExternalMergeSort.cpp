#include "ExternalMergeSort.h"

#ifdef EXTERNAL_MERGE_SORT_LOG
#include <math.h>
#endif

static inline bool load(int* mem, FILE* input_file, size_t begin, size_t& end) {
		while (fscanf(input_file, "%d", mem + (end++))!=EOF) {
		if (end == begin + BLOCK_SIZE) {
			return 0;
		}
	}
	end--;
	return 1;
}
static inline void store(FILE* output_file, int* mem, size_t begin, size_t end) {
	for (int itr = begin; itr != end; ++itr) {
		fprintf(output_file, "%d\n", mem[itr]);
	}
}
void externalMergeSort(FILE* input, FILE* output) {
#ifdef DISK_MONITOR
	size_t disk_times = 0;//寻道数
	size_t block_cnt = 0;//传输块数
#endif
#ifdef EXTERNAL_MERGE_SORT_LOG
	size_t sort_size = 0;//排序元素个数
	
	size_t init_op_tree_cnt = 0;//初始化归并段树操作数
	size_t init_op_tree_init_cnt = 0;//初始化归并段树初始化数

	size_t merge_op_tree_cnt = 0;//归并时树操作次数
	size_t merge_op_tree_init_cnt = 0;//归并时树初始化次数
#endif
	int* memory = new int[MEMORY_SIZE];
	size_t mem_ptr = 0;

	char* file_name = new char[100];
	size_t segments_cnt = 0;
	Lqueue<size_t> que;
	//初始化归并段 
	while (fscanf(input, "%d", memory + (mem_ptr++)) != EOF) {
#ifdef EXTERNAL_MERGE_SORT_LOG
		//每加载一个元素
		sort_size++;//排序元素个数
#endif
		if (mem_ptr == MEMORY_SIZE) {
#ifdef DISK_MONITOR
			//读入寻道一次，写出寻到一次
			disk_times+=2;
			//传输块数，两倍主存
			block_cnt += 2 * (BLOCK_COUNT);
#endif
#ifdef EXTERNAL_MERGE_SORT_LOG
			//对主存里的数排序
			init_op_tree_cnt+=MEMORY_SIZE;//树元素操作次数
			init_op_tree_init_cnt++;//树初始化次数
#endif
			MinLoserTree<int> loserTree(memory, MEMORY_SIZE);
			for (size_t i = 0; i < MEMORY_SIZE; ++i) {
				memory[i] = loserTree.top();
				loserTree.change_top(INT_MAX);
			}

			que.push(segments_cnt);
			sprintf(file_name, "disk/Merge Segments #%llu", segments_cnt++);
			FILE* fp = fopen(file_name, "w");
			for (size_t i = 0; i < MEMORY_SIZE; ++i) {
				fprintf(fp, "%d\n", memory[i]);
			}
			fclose(fp);
			mem_ptr = 0;
		}
	}
	if (--mem_ptr) {//还剩一点
#ifdef DISK_MONITOR
		//读入寻道一次，写出寻到一次
		disk_times += 2;
		//传输块数，两倍
		block_cnt += 2 * (mem_ptr/BLOCK_SIZE);
#endif
#ifdef EXTERNAL_MERGE_SORT_LOG
		//对主存里的数排序
		init_op_tree_cnt += mem_ptr;//树元素操作次数
		init_op_tree_init_cnt++;//树初始化次数
#endif
		MinLoserTree<int> loserTree(memory, mem_ptr);
		for (size_t i = 0; i < mem_ptr; ++i) {
			memory[i] = loserTree.top();
			loserTree.change_top(INT_MAX);
		}

		que.push(segments_cnt);
		sprintf(file_name, "disk/Merge Segments #%llu", segments_cnt++);
		FILE* fp = fopen(file_name, "w");
		for (size_t i = 0; i < mem_ptr; ++i) {
			fprintf(fp, "%d\n", memory[i]);
		}
		fclose(fp);
		mem_ptr = 0;
	}
	//开始合并
	FILE** merge_fp = new FILE * [BLOCK_COUNT - 1];//用于存储每个归并段的指针
	bool* is_file_over = new bool[BLOCK_COUNT - 1];
	bool* is_data_over = new bool[BLOCK_COUNT - 1];

	size_t* merge_begin = new size_t[BLOCK_COUNT - 1];
	size_t* merge_itr = new size_t[BLOCK_COUNT - 1];
	size_t* merge_end = new size_t[BLOCK_COUNT - 1];

	size_t merge_fp_size = 0;
	size_t using_cnt = 0;

	const size_t buffer_begin = (BLOCK_COUNT - 1) * BLOCK_SIZE;
	const size_t buffer_end = buffer_begin + BLOCK_SIZE;
	size_t buffer_itr = buffer_begin;

	while (que.size() > 1) {
		//reset
		merge_fp_size = 0;
		using_cnt = 0;

		//fopen		
		//*** 从队列中取出归并段，往merge_fp里塞
		//*** 并读一个块(可能读到尾，加标记)到内存（更新begin_end）, 
		for (int i = 0; i < BLOCK_COUNT - 1; ++i) {
			if (!que.empty()) {
				sprintf(file_name, "disk/Merge Segments #%llu", que.front());
				que.pop();
				merge_fp[merge_fp_size] = fopen(file_name, "r");
				is_file_over[merge_fp_size] = 0;
				is_data_over[merge_fp_size] = 0;
				merge_begin[merge_fp_size] = i * BLOCK_SIZE;
				merge_end[merge_fp_size] = merge_begin[merge_fp_size];
#ifdef DISK_MONITOR
				//load一次寻道一次
				disk_times += 1;
				//传输一块
				block_cnt += 1;
#endif
				bool eof = load(memory, merge_fp[merge_fp_size], merge_begin[merge_fp_size], merge_end[merge_fp_size]);
				merge_itr[merge_fp_size] = merge_begin[merge_fp_size];
				if (eof) {
					//TODO file end
					fclose(merge_fp[merge_fp_size]);
					is_file_over[merge_fp_size] = 1;
				}
				merge_fp_size++;
				using_cnt++;
			}
			else {
				break;
			}
		}
		//***完了建输者树, 用pair
		std::pair<int, int>* ltree_init = new std::pair<int, int>[merge_fp_size];

		for (int i = 0; i < merge_fp_size; ++i) {
#ifdef ___EXTERNAL_MERGE_SORT_VIEW__
			printf("init: %d \n", memory[merge_itr[i]]);
#endif
			ltree_init[i] = {memory[merge_itr[i]++], i};
			if (merge_itr[i] == merge_end[i]) {
				//TODO block end
				if (!is_file_over[i]) {
#ifdef DISK_MONITOR
					//load一次寻道一次
					disk_times += 1;
					//传输一块
					block_cnt += 1;
#endif
					merge_end[i] = merge_begin[i];
					bool eof = load(memory, merge_fp[i], merge_begin[i], merge_end[i]);
					merge_itr[i] = merge_begin[i];
					if (eof) {
						//TODO file end
						fclose(merge_fp[i]);
						is_file_over[i] = 1;							
						//load直接是data over
						if (merge_begin[i] == merge_end[i]) {
							is_data_over[i] = 1;
						}
					}
				}
				else {
					is_data_over[i] = 1;
				}
			}
		}
#ifdef EXTERNAL_MERGE_SORT_LOG
		merge_op_tree_init_cnt++;//树初始化次数
#endif
		MinLoserTree<std::pair<int, int>> ltree(ltree_init, merge_fp_size);
		delete[] ltree_init;
		//*** 每次从输者树取一个到buffer（buffer满了写），再从对应块取一个数(begin++)塞回去（块空了(==end)再load）(load到尾更新over)
		
		//创建输出归并段
		que.push(segments_cnt);
		sprintf(file_name, "disk/Merge Segments #%llu", segments_cnt++);
		FILE* new_fp = fopen(file_name, "w");

		while (using_cnt) {
			//树里取数存buffer
			int now_data = ltree.top().first;
			int now_idx = ltree.top().second;

			memory[buffer_itr++] = now_data;
			if (buffer_itr == buffer_end) {
#ifdef DISK_MONITOR
				//store一次寻道一次
				disk_times += 1;
				//传输一块
				block_cnt += 1;
#endif				//输出，重置buffer
				store(new_fp, memory, buffer_begin, buffer_end);
				buffer_itr = buffer_begin;
			}
			//主存取数存树里
			if (is_data_over[now_idx]) {
#ifdef _EXTERNAL_MERGE_SORT_VIEW__
				printf("get:%d to INF [%d]\n", now_data, now_idx);
#endif
#ifdef EXTERNAL_MERGE_SORT_LOG
				merge_op_tree_cnt++;
#endif
				ltree.change_top({ INT_MAX, now_idx });
				using_cnt--;
			}
			else {
#ifdef _EXTERNAL_MERGE_SORT_VIEW__
				printf("get:%d to %d [%d]\n", now_data, memory[merge_itr[now_idx]], now_idx);
#endif
#ifdef EXTERNAL_MERGE_SORT_LOG
				merge_op_tree_cnt++;
#endif
				ltree.change_top({ memory[merge_itr[now_idx]++], now_idx });
				//主存块 处理
				if (merge_itr[now_idx] == merge_end[now_idx]) {
					//block end
					if (!is_file_over[now_idx]) {
#ifdef DISK_MONITOR
						//load一次寻道一次
						disk_times += 1;
						//传输一块
						block_cnt += 1;
#endif
						merge_end[now_idx] = merge_begin[now_idx];
						bool eof = load(memory, merge_fp[now_idx], merge_begin[now_idx], merge_end[now_idx]);
						merge_itr[now_idx] = merge_begin[now_idx];
						if (eof) {
							//file end
							fclose(merge_fp[now_idx]);
							is_file_over[now_idx] = 1;
							//load直接是data over
							if (merge_begin[now_idx] == merge_end[now_idx]) {
								is_data_over[now_idx] = 1;
							}
						}
					}
					else {
						is_data_over[now_idx] = 1;
					}
				}
			}
		}
		//清buffer
		store(new_fp, memory, buffer_begin, buffer_itr);
		buffer_itr = buffer_begin;
#ifdef DISK_MONITOR
		//store一次寻道一次
		disk_times += 1;
		//传输一块
		block_cnt += 1;
#endif		
		fclose(new_fp);
	}
	//队列最后一个是最终文件
	sprintf(file_name, "disk/Merge Segments #%llu", que.front());
	FILE* fp = fopen(file_name, "r");
	while (fscanf(fp, "%d", memory) != EOF) {
		fprintf(output, "%d\n", *memory);
	}
	fclose(fp);

	delete[] merge_fp;
	delete[] is_file_over;
	delete[] is_data_over;
	delete[] merge_begin;
	delete[] merge_itr;
	delete[] merge_end;

	delete[] file_name;
	delete[] memory;

#ifdef _EXTERNAL_MERGE_SORT_VIEW__
#ifdef DISK_MONITOR
	printf("-------disk monitor--------\n");
	printf("disk times:%llu\n", disk_times);
	printf("dick block:%llu\n", block_cnt);
	printf("---------------------------\n");
#endif
#ifdef EXTERNAL_MERGE_SORT_LOG
	printf("-------sort log-----------\n");
	printf("sort size:%llu\n", sort_size);
	printf("init op tree cnt:%llu\n", init_op_tree_cnt);
	printf("init op tree init cnt:%llu\n", init_op_tree_init_cnt);
	printf("merge op tree cnt:%llu\n", merge_op_tree_cnt);
	printf("merge op tree init cnt:%llu\n", merge_op_tree_init_cnt);
	printf("--------------------------\n");
#endif
#endif

#ifdef EXTERNAL_MERGE_SORT_LOG
	FILE* log = fopen("disk/log.txt", "w");
	fprintf(log, "M:%d\n", MEMORY_SIZE);
	fprintf(log, "B:%d\n", BLOCK_SIZE);
	fprintf(log, "C:%d\n", BLOCK_COUNT);
#ifdef DISK_MONITOR
	fprintf(log, "-------disk monitor--------\n");
	fprintf(log, "disk times:%llu\n", disk_times);
	fprintf(log, "dick block:%llu\n", block_cnt);
	fprintf(log, "---------------------------\n");
#endif
	fprintf(log, "-------sort log-----------\n");
	fprintf(log, "sort size:%llu\n", sort_size);
	fprintf(log, "init op tree cnt:%llu\n", init_op_tree_cnt);
	fprintf(log, "init op tree init cnt:%llu\n", init_op_tree_init_cnt);
	fprintf(log, "merge op tree cnt:%llu\n", merge_op_tree_cnt);
	fprintf(log, "merge op tree init cnt:%llu\n", merge_op_tree_init_cnt);
	fprintf(log, "--------------------------\n");
	
	fprintf(log, "-------analysis-----------\n");
	size_t all_op_cnt =
		init_op_tree_init_cnt * 2 * MEMORY_SIZE +
		init_op_tree_cnt * (log2(MEMORY_SIZE)) +
		merge_op_tree_init_cnt * 2 * (BLOCK_COUNT - 1) +
		merge_op_tree_cnt * (log2(BLOCK_COUNT - 1));
	fprintf(log, "all op cnt:%llu\n", all_op_cnt);
	fprintf(log, "--------------------------\n");

	fclose(log);
#endif
}