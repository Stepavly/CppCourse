#ifndef VECTOR_H
#define VECTOR_H

#include <cstddef>
#include <cstring>
#include <cassert>
#include <utility>

template<typename T>
struct vector {
	typedef T *iterator;
	typedef T const *const_iterator;

	vector();                               // O(1) nothrow
	vector(vector const &);                  // O(N) strong
	vector &operator=(vector const &other); // O(N) strong

	~vector();                              // O(N) nothrow

	T &operator[](size_t i);                // O(1) nothrow
	T const &operator[](size_t i) const;    // O(1) nothrow

	T *data();                              // O(1) nothrow
	T const *data() const;                  // O(1) nothrow
	size_t size() const;                    // O(1) nothrow

	T &front();                             // O(1) nothrow
	T const &front() const;                 // O(1) nothrow

	T &back();                              // O(1) nothrow
	T const &back() const;                  // O(1) nothrow
	void push_back(T const &);               // O(1)* strong
	void pop_back();                        // O(1) nothrow

	bool empty() const;                     // O(1) nothrow

	size_t capacity() const;                // O(1) nothrow
	void reserve(size_t);                   // O(N) strong
	void shrink_to_fit();                   // O(N) strong

	void clear();                           // O(N) nothrow

	void swap(vector &);                     // O(1) nothrow

	iterator begin();                       // O(1) nothrow
	iterator end();                         // O(1) nothrow

	const_iterator begin() const;           // O(1) nothrow
	const_iterator end() const;             // O(1) nothrow

	iterator insert(iterator pos, T const &); // O(N) weak
	iterator insert(const_iterator pos, T const &); // O(N) weak

	iterator erase(iterator pos);           // O(N) weak
	iterator erase(const_iterator pos);     // O(N) weak

	iterator erase(iterator first, iterator last); // O(N) weak
	iterator erase(const_iterator first, const_iterator last); // O(N) weak

 private:
	void push_back_realloc(T const &);
	void clear_buffer();

 private:
	T *data_;
	T *new_data_buffer_;
	size_t size_;
	size_t capacity_;
	size_t new_data_size_;
};

template<typename T>
vector<T>::vector() {                       // O(1) nothrow
	size_ = 0;
	capacity_ = 0;
	data_ = static_cast<T*>(operator new(capacity_ * sizeof(T)));
	new_data_buffer_ = nullptr;
}
template<typename T>
vector<T>::vector(vector<T> const &rhs) {      // O(N) strong
	new_data_buffer_ = static_cast<T *>(operator new(rhs.capacity_ * sizeof(T)));
	new_data_size_ = 0;

	for (size_t i = 0; i < rhs.size_; i++) {
		new(new_data_buffer_ + i) T(rhs.data_[i]);
		new_data_size_++;
	}

	size_ = rhs.size_;
	capacity_ = rhs.capacity_;
	std::swap(new_data_buffer_, data_);
	new_data_size_ = 0;
	new_data_buffer_ = nullptr;
}
template<typename T>
vector<T> &vector<T>::operator=(vector<T> const &other) {
	vector<T> temp(other);
	swap(temp);
	return *this;
}

template<typename T>
vector<T>::~vector() {
	clear();
	size_ = 0;
	capacity_ = 0;
	clear_buffer();
	operator delete(data_);
	data_ = nullptr;
}

template<typename T>
T &vector<T>::operator[](size_t i) {
	assert(i < size_);
	return data_[i];
}

template<typename T>
T const &vector<T>::operator[](size_t i) const {
	assert(i < size_);
	return data_[i];
}

template<typename T>
T *vector<T>::data() {
	return capacity_ > 0 ? data_ : nullptr;
}

template<typename T>
T const *vector<T>::data() const {
	return capacity_ > 0 ? data_ : nullptr;
}

template<typename T>
size_t vector<T>::size() const {
	return size_;
}

template<typename T>
T &vector<T>::front() {
	return *data_;
}

template<typename T>
T const &vector<T>::front() const {
	return *data_;
}

template<typename T>
T &vector<T>::back() {
	assert(size_ > 0);
	return data_[size_ - 1];
}
template<typename T>
T const &vector<T>::back() const {
	assert(size_ > 0);
	return data_[size_ - 1];
}

template<typename T>
void vector<T>::push_back(T const &value) {
	if (size_ != capacity_) {
		new(data_ + size_) T(value);
		size_++;
	} else {
		push_back_realloc(T(value));
	}
}
template<typename T>
void vector<T>::pop_back() {
	assert(size_ > 0);
	data_[--size_].~T();
}

template<typename T>
bool vector<T>::empty() const {
	return size_ == 0;
}

template<typename T>
size_t vector<T>::capacity() const {
	return capacity_;
}
template<typename T>
void vector<T>::reserve(size_t n) {
	if (n > capacity_) {
		clear_buffer();
		new_data_buffer_ = static_cast<T *>(operator new(n * sizeof(T)));
		new_data_size_ = 0;

		for (size_t i = 0; i < size_; i++) {
			new(new_data_buffer_ + i) T(data_[i]);
			new_data_size_++;
		}

		std::swap(data_, new_data_buffer_);

		clear_buffer();
		capacity_ = n;
	}
}
template<typename T>
void vector<T>::shrink_to_fit() {
	if (capacity_ == size_) {
		return;
	}
	clear_buffer();
	new_data_buffer_ = static_cast<T *>(operator new(size_ * sizeof(T)));
	new_data_size_ = 0;

	for (size_t i = 0; i < size_; i++) {
		new(new_data_buffer_ + i) T(data_[i]);
		new_data_size_++;
	}

	std::swap(data_, new_data_buffer_);
	clear_buffer();
	capacity_ = size_;
}

template<typename T>
void vector<T>::clear() {
	for (size_t i = 0; i < size_; i++) {
		data_[i].~T();
	}
	size_ = 0;
	clear_buffer();
}

template<typename T>
void vector<T>::swap(vector<T> &other) {
	std::swap(data_, other.data_);
	std::swap(size_, other.size_);
	std::swap(capacity_, other.capacity_);
}

template<typename T>
typename vector<T>::iterator vector<T>::begin() {
	return data_;
}
template<typename T>
typename vector<T>::iterator vector<T>::end() {
	return data_ + size_;
}

template<typename T>
typename vector<T>::const_iterator vector<T>::begin() const {
	return data_;
}
template<typename T>
typename vector<T>::const_iterator vector<T>::end() const {
	return data_ + size_;
}

template<typename T>
typename vector<T>::iterator vector<T>::insert(iterator pos, T const &value) {
	size_t pos_ = pos - begin();
	push_back(value);

	for (size_t i = size_ - 1; i != pos_; i--) {
		std::swap(data_[i], data_[i - 1]);
	}

	return begin() + pos_;
}
template<typename T>
typename vector<T>::iterator vector<T>::insert(const_iterator pos, T const &value) {
	size_t pos_ = pos - begin();
	push_back(value);

	for (size_t i = size_ - 1; i != pos_; i--) {
		std::swap(data_[i], data_[i - 1]);
	}

	return begin() + pos_;
}

template<typename T>
typename vector<T>::iterator vector<T>::erase(iterator pos) {
	size_t pos_ = pos - begin();

	for (size_t i = pos_; i + 1 < size_; i++) {
		std::swap(data_[i], data_[i + 1]);
	}

	pop_back();
	return begin() + pos_;
}
template<typename T>
typename vector<T>::iterator vector<T>::erase(const_iterator pos) {
	size_t pos_ = pos - begin();

	for (size_t i = pos_; i + 1 < size_; i++) {
		std::swap(data_[i], data_[i + 1]);
	}

	pop_back();
	return begin() + pos_;
}

template<typename T>
typename vector<T>::iterator vector<T>::erase(iterator first, iterator last) {
	assert(first < last);
	size_t begin_ = first - begin();
	size_t end_ = last - begin();
	size_t delta_ = end_ - begin_;

	for (size_t i = end_; i < size_; i++) {
		std::swap(data_[i - delta_], data_[i]);
	}

	for (size_t i = 0; i < delta_; i++) {
		pop_back();
	}

	return begin() + begin_;
}
template<typename T>
typename vector<T>::iterator vector<T>::erase(const_iterator first, const_iterator last) {
	assert(first < last);
	size_t begin_ = first - begin();
	size_t end_ = last - begin();
	size_t delta_ = end_ - begin_;

	for (size_t i = end_; i < size_; i++) {
		std::swap(data_[i - delta_], data_[i]);
	}

	for (size_t i = 0; i < delta_; i++) {
		pop_back();
	}

	return begin() + begin_;
}

template<typename T>
void vector<T>::push_back_realloc(T const &other) {
	reserve(2 * capacity_ + 1);
	new(data_ + size_++) T(other);
}
template<typename T>
void vector<T>::clear_buffer() {
	if (new_data_buffer_) {
		for (size_t i = 0; i < new_data_size_; i++) {
			new_data_buffer_[i].~T();
		}

		operator delete(new_data_buffer_);
		new_data_buffer_ = nullptr;
	}
}
#endif // VECTOR_H