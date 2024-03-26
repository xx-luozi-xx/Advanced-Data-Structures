#pragma once
#include <vector>
#include <list>			
#include <tuple>		//pair
#include <algorithm>	//sort, random_shuffle
#include <iostream>


template<typename T>
class SparseMatrix {//! 维度表示[行][列], 矩阵下标0开始, 如3*5矩阵不能访问[3][5]
public:
	struct SparseMatrix_err;
public:
	SparseMatrix(size_t rows, size_t cols)noexcept;
	SparseMatrix(size_t row, size_t col, const std::vector<std::pair<std::pair<size_t, size_t>, T>>& data);
	SparseMatrix(const SparseMatrix<T>& rhs);
	SparseMatrix<T>& operator=(const SparseMatrix<T>& rhs);
	//todo 右值引用不会
	~SparseMatrix() = default;

	void swap(SparseMatrix<T>& rhs) noexcept;

	size_t cols()const noexcept;
	size_t rows()const noexcept;

	SparseMatrix<T> transposition()const noexcept;

	SparseMatrix<T> operator * (const SparseMatrix<T>& rhs) const;

	void add_rows(const SparseMatrix<T>& rhs);

	std::vector<std::vector<T>> to_vector()const noexcept;

public:
	static SparseMatrix<T> I(size_t n)noexcept;
	//! solute() is only double could use
	static SparseMatrix<T> solute(SparseMatrix<T> A, SparseMatrix<T> B); //return X, S.T. AX = B
	template<typename T>
	friend std::ostream& operator << (std::ostream& out, const SparseMatrix<T>& rhs) noexcept;
	template<typename T>
	friend SparseMatrix<T> operator * (T lhs, const SparseMatrix<T>& rhs) noexcept;

public:
	struct SparseMatrix_err {
		enum RC {
			//nagative
			INIT_ROW_ERR = 1,
			INIT_COL_ERR,
			//out of bound
			INIT_ELEMENT_ROW_ERR,
			INIT_ELEMENT_COL_ERR,

			INIT_COORDINATE_REPETITION,
			//calculate
			MATRIX_MULTIPLICATION_INCOMPATIBILITY,
			MATRIX_SOLUTE_INCOMPATIBILITY,
			UNSOLVABLE,

			//merge
			MATRIX_MERGE_COL_INCOMPATIBILITY,
		} rc;
		SparseMatrix_err(RC rc)noexcept :rc(rc) { std::cerr << "SparseMatrix error #" << rc << std::endl; }
	};
private:
	struct Term {
		Term(size_t col, T data)noexcept :col(col), data(data) {}
		size_t col;
		T data;
	};

protected:
	size_t rows_;
	size_t cols_;
	std::vector<std::list<Term>> data_;

private:
	static void row_muti(std::list<Term>& row, T rhs)noexcept;
	static void row_add(std::list<Term>& row, const std::list<Term>& rhs, T k)noexcept;
};

template<typename T>
inline SparseMatrix<T>::SparseMatrix(size_t row, size_t col, const std::vector<std::pair<std::pair<size_t, size_t>, T>>& data)
	:rows_(row), cols_(col), data_(row, std::list<Term>()) {
	if (row <= 0) { throw SparseMatrix_err(SparseMatrix_err::RC::INIT_ROW_ERR); }
	if (col <= 0) { throw SparseMatrix_err(SparseMatrix_err::RC::INIT_COL_ERR); }

	std::vector<std::pair<std::pair<size_t, size_t>, T>> init(data);

	std::random_shuffle(init.begin(), init.end());
	std::sort(init.begin(), init.end());

	for (size_t i = 0; i < init.size(); ++i) {
		if (init[i].first.first < 0 || row <= init[i].first.first) { throw SparseMatrix_err(SparseMatrix_err::RC::INIT_ELEMENT_ROW_ERR); }
		if (init[i].first.second < 0 || col <= init[i].first.second) { throw SparseMatrix_err(SparseMatrix_err::RC::INIT_ELEMENT_COL_ERR); }
		if (init[i].second != T()) {
			if (!data_[init[i].first.first].empty() && data_[init[i].first.first].back().col == init[i].first.second) {
				throw SparseMatrix_err(SparseMatrix_err::RC::INIT_COORDINATE_REPETITION);
			}
			data_[init[i].first.first].emplace_back(Term(init[i].first.second, init[i].second));
		}
	}
}
template<typename T>
inline SparseMatrix<T>::SparseMatrix(const SparseMatrix<T>& rhs)
	:rows_(rhs.rows_), cols_(rhs.cols_), data_(rhs.data_) {}
template<typename T>
typename SparseMatrix<T>& SparseMatrix<T>::operator=(const SparseMatrix<T>& rhs) {
	SparseMatrix<T> tmp(rhs);
	this->swap(tmp);
	return *this;
}
template<typename T>
inline void SparseMatrix<T>::swap(SparseMatrix<T>& rhs)noexcept {
	std::swap(rows_, rhs.rows_);
	std::swap(cols_, rhs.cols_);
	data_.swap(rhs.data_);
}
template<typename T>
inline size_t SparseMatrix<T>::cols()const noexcept {
	return cols_;
}
template<typename T>
inline size_t SparseMatrix<T>::rows()const noexcept {
	return rows_;
}
template<typename T>
inline typename SparseMatrix<T> SparseMatrix<T>::transposition()const noexcept {
	SparseMatrix<T> ret(cols_, rows_);
	for (size_t i = 0; i < data_.size(); ++i) {
		for (auto itr = data_[i].begin(); itr != data_[i].end(); ++itr) {
			ret.data_[itr->col].emplace_back(Term(i, itr->data));
		}
	}
	return ret;
}
template<typename T>
inline SparseMatrix<T>::SparseMatrix(size_t rows, size_t cols)noexcept
	:rows_(rows), cols_(cols), data_(rows, std::list<Term>()) {}

template<typename T>
std::ostream& operator << (std::ostream& out, const SparseMatrix<T>& rhs) noexcept {
	out << "row:" << rhs.rows_ << ' ' << "cols:" << rhs.cols_ << '\n';
	for (size_t i = 0; i < rhs.rows_; ++i) {
		auto itr = rhs.data_[i].begin();
		for (size_t j = 0; j < rhs.cols_; ++j) {
			if (itr != rhs.data_[i].end()) {
				if (itr->col == j) {
					out << itr->data << ' ';
					++itr;
				}
				else {
					out << T() << ' ';
				}
			}
			else {
				out << T() << ' ';
			}
		}
		out << '\n';
	}
	return out;
}
template<typename T>
inline typename SparseMatrix<T> operator * (T lhs, const SparseMatrix<T>& rhs) noexcept {
	SparseMatrix<T> ret(rhs);
	for (size_t i = 0; i < ret.data_.size(); ++i) {
		for (auto itr = ret.data_[i].begin(); itr != ret.data_[i].end(); ++itr) {
			itr->data *= lhs;
		}
	}
	return ret;
}
template<typename T>
typename SparseMatrix<T> SparseMatrix<T>::operator * (const SparseMatrix<T>& rhs) const {
	if (cols_ != rhs.rows_) { throw SparseMatrix_err(SparseMatrix_err::RC::MATRIX_MULTIPLICATION_INCOMPATIBILITY); }
	SparseMatrix<T> ret(rows_, rhs.cols_);
	SparseMatrix<T> rhsT = rhs.transposition();

	for (size_t i = 0; i < rows_; ++i) {
		for (size_t j = 0; j < rhsT.rows_; ++j) {
			T sum = T();
			auto itr_l = data_[i].begin();
			auto itr_r = rhsT.data_[j].begin();
			while (itr_l != data_[i].end() && itr_r != rhsT.data_[j].end()) {
				if (itr_l->col == itr_r->col) {
					sum += itr_l->data * itr_r->data;
					++itr_l, ++itr_r;
				}
				else if (itr_l->col < itr_r->col) {
					++itr_l;
				}
				else {
					++itr_r;
				}
			}
			if (sum != T()) {
				ret.data_[i].emplace_back(Term(j, sum));
			}
		}
	}
	return ret;
}
template<typename T>
inline typename SparseMatrix<T> SparseMatrix<T>::I(size_t n)noexcept {
	SparseMatrix<T> ret(n, n);
	for (size_t i = 0; i < n; ++i) {
		ret.data_[i].emplace_back(Term(i, T(1)));
	}
	return ret;
}
template<typename T>
typename SparseMatrix<T> SparseMatrix<T>::solute(SparseMatrix<T> A, SparseMatrix<T> B) {//return X, S.T. AX = B
	if(A.rows_ != B.rows_){ throw SparseMatrix_err(SparseMatrix_err::RC::MATRIX_SOLUTE_INCOMPATIBILITY); }

	std::vector<std::list<Term>::iterator> itrs;
	std::vector<size_t> main_cols;
	main_cols.reserve(A.cols_);
	itrs.reserve(A.rows_);

	for(size_t i = 0; i < A.rows_; ++i){
		itrs.emplace_back(A.data_[i].begin());
	}
	
	size_t do_rows = 0;
	for(size_t i = 0; i < A.cols_ && do_rows < A.rows_; ++i){//消每一列
		std::clog << "(" << i << "/" << A.cols_<< ")";
		size_t get = -1;
		for(size_t j = do_rows; j < A.rows_; ++j){//在第i列，找用于消除的行
			while(itrs[j]!=A.data_[j].end() && itrs[j]->col < i){
				++itrs[j];
			}
			if(itrs[j]!=A.data_[j].end() && itrs[j]->col == i){//! 不需判0
				get = j;
				std::swap(itrs[j], itrs[do_rows]);
				A.data_[j].swap(A.data_[do_rows]);
				B.data_[j].swap(B.data_[do_rows]);
				break;
			}
		}
		if(get != size_t(-1)){//找到行了，开消
			T k = 1/(itrs[do_rows]->data);
			row_muti(A.data_[do_rows], k);
			row_muti(B.data_[do_rows], k);
			for(size_t j = do_rows+1; j < A.rows_; ++j){
				while(itrs[j]!=A.data_[j].end() && itrs[j]->col < i){
					++itrs[j];
				}
				if(itrs[j]!=A.data_[j].end() && itrs[j]->col == i){
					T k = -itrs[j]->data;
					row_add(A.data_[j], A.data_[do_rows], k);
					row_add(B.data_[j], B.data_[do_rows], k);
					itrs[j] = A.data_[j].begin();
				}
			}
			main_cols.emplace_back(i);
			do_rows++;
		}else{//* the col is out of rank
			void();
		}
	}
	//回消去
	std::vector<std::reverse_iterator<std::list<Term>::iterator>> ritrs;
	ritrs.reserve(A.rows_);
	for(size_t i = 0; i < A.rows_; ++i){
		ritrs.emplace_back(A.data_[i].rbegin());
	}

	for (size_t i = A.rows_ - 1; i != size_t(-1); --i) {
		std::clog << "(" << i << "/" << A.cols_<< ")";
		if(!A.data_[i].empty()){//*用第i行删其他行
			for(size_t j = 0; j < i; ++j){//* 第j行被删
				while(ritrs[j]!=A.data_[j].rend() && ritrs[j]->col > A.data_[i].begin()->col){
					++ritrs[j];
				}
				if(ritrs[j]!=A.data_[j].rend() && ritrs[j]->col == A.data_[i].begin()->col){
					T k = -ritrs[j]->data;
					row_add(A.data_[j], A.data_[i], k);
					row_add(B.data_[j], B.data_[i], k);
					ritrs[j] = A.data_[j].rbegin();					
				}
			}
		}
	}

	SparseMatrix<T> ret(A.rows_, B.cols_);
	for(int i = 0; i < main_cols.size(); ++i){
		ret.data_[main_cols[i]].swap(B.data_[i]);
	}
	//todo check unsoluvabel

	return ret;
}

template<typename T>
void SparseMatrix<T>::row_muti(std::list<Term>& row, T rhs)noexcept{
	if(rhs == T()){
		row.clear();
	}else{
		for(auto itr = row.begin(); itr != row.end(); ++itr){
			itr->data *= rhs;
		}
	}
}
template<typename T>
void SparseMatrix<T>::row_add(std::list<Term>& row, const std::list<Term>& rhs, T k)noexcept{
	std::list<Term> ret;
	auto itr1 = row.begin();
	auto itr2 = rhs.begin();
	while(itr1 != row.end() && itr2 != rhs.end()){
		if(itr1->col == itr2->col){
			T data = itr1->data + (itr2->data * k);
			if(data != T()){
				ret.emplace_back(Term(itr1->col, data));
			}
			++itr1;
			++itr2;
		}else if(itr1->col < itr2->col){
			ret.emplace_back(*itr1);
			++itr1;
		}else{
			if(itr2->data * k != T()){
				ret.emplace_back(Term(itr2->col, itr2->data * k));
			}
			++itr2;
		}
	}
	while(itr1 != row.end()){
		ret.emplace_back(*itr1);
		++itr1;
	}
	while(itr2 != rhs.end()){
		if(itr2->data * k != T()){
			ret.emplace_back(Term(itr2->col, itr2->data * k));
		}
		++itr2;
	}
	row.swap(ret);
}
template<typename T>
typename std::vector<std::vector<T>> SparseMatrix<T>::to_vector()const noexcept{
	std::vector<std::vector<T>> ret;
	ret.reserve(rows_);
	for (size_t i = 0; i < rows_; ++i) {
		ret.emplace_back(std::vector<T>());
		ret[i].reserve(cols_);
		auto itr = data_[i].begin();
		for (size_t j = 0; j < cols_; ++j) {
			if (itr != data_[i].end()) {
				if (itr->col == j) {
					ret[i].emplace_back(itr->data);
					++itr;
				}else {
					ret[i].emplace_back(T());
				}
			}
			else {
				ret[i].emplace_back(T());
			}
		}
	}
	return ret;
}
template<typename T>
void SparseMatrix<T>::add_rows(const SparseMatrix<T>& rhs){
	if(cols_!=rhs.cols_){throw SparseMatrix_err(SparseMatrix_err::RC::MATRIX_MERGE_COL_INCOMPATIBILITY);}
	rows_+=rhs.rows_;
	data_.insert(data_.end(), rhs.data_.begin(), rhs.data_.end());
}
