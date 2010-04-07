/** 
 * @file sparse_matrix.hpp
 * @brief base class for sparse matrices in DIA format
 * @ingroup basics
 * @author Hannes Schulz
 * @date 2010-03-21
 */
#ifndef __SPARSE_MATRIX_HPP__
#define __SPARSE_MATRIX_HPP__
#include <vector>
#include <map>
#include <memory>
#include <iostream>
#include <vector.hpp>
#include <vector_ops/vector_ops.hpp>
#include <matrix.hpp>
#include <cuv_general.hpp>

namespace cuv{
	/** 
	 * @brief Parent class for diagonal matrices
	 */
	template<class __value_type, class __index_type, class __vec_type, class __intvec_type> 
	class dia_matrix 
	:        public matrix<__value_type, __index_type>{
	  public:
		  typedef __vec_type    vec_type; ///< Basic vector type of matrix
		  typedef __intvec_type intvec_type; ///< Type of offsets for diagonals
		  typedef matrix<__value_type, __index_type> base_type; ///< Basic matrix type
		  typedef typename base_type::value_type value_type;///< Type of matrix entries
		  typedef typename base_type::index_type index_type;///< Type of indices
		//protected:
		public:
		  int m_num_dia;                        ///< number of diagonals stored
		  int m_stride;                         ///< how long the stored diagonals are
		  vec_type* m_vec;                      ///< stores the actual data 
		  intvec_type m_offsets;                ///< stores the offsets of the diagonals
		  std::map<int,index_type> m_dia2off;   ///< maps a diagonal to an offset
		  int m_row_fact;                       ///< factor by which to multiply a row index (allows matrices with "steep" diagonals)
		public:
			dia_matrix() ///< Empty constructor. Returns empty diagonal matrix.
				: base_type(0,0),
				 m_vec(0),
				 m_num_dia(0),
				 m_stride(0),
				 m_row_fact(0){}
			/** 
			 * @brief Creates diagonal matrix of given size, with given number of diagonals and stride.
			 * 
			 * @param h Height of matrix 
			 * @param w Width of matrix
			 * @param num_dia Number of diagonals in matrix
			 * @param stride Stride of matrix
			 * @param row_fact Steepness of diagonals. Only 1 is supported at the moment.
			 */
			dia_matrix(const index_type& h, const index_type& w, const int& num_dia, const int& stride, int row_fact=1)
				: base_type(h,w)
				, m_num_dia(num_dia)
				, m_stride(stride)
				, m_offsets(num_dia)
			{
				m_row_fact = row_fact;
				cuvAssert(m_row_fact>0);
				alloc();
			}
			//~dia_matrix() ///< Deallocate matrix 
			//{
				//dealloc();
			//}
			void dealloc() ///< Deallocate matrix entries. This calls deallocation of the vector storing entries.
			{
				if(m_vec)
					delete m_vec;
				m_vec = NULL;
			}
			void alloc() ///< Allocate matrix entries: Create vector to store entries.
			{
				cuvAssert(m_stride >= this->h() || m_stride >= this->w());
				m_vec = new vec_type(m_stride * m_num_dia);
			}
			inline const vec_type& vec()const{ return *m_vec; } ///< Return pointer to vector storing entries
			inline       vec_type& vec()     { return *m_vec; } ///< Return pointer to vector storing entries
			inline const vec_type* vec_ptr()const{ return m_vec; } ///< Return reference to vector storing entries
			inline       vec_type* vec_ptr()     { return m_vec; } ///< Return reference to vector storing entries
			inline int num_dia()const{ return m_num_dia; } ///< Return number of diagonals
			inline int stride()const { return m_stride;  }///< Return stride of matrix
			inline int row_fact()const{ return m_row_fact; } ///< Return steepness of diagonals

			//*****************************
			// set/get offsets of diagonals
			//*****************************
		
			template<class T>
			void set_offsets(T a, const T& b){
				int i=0;
				while(a!=b)
					m_offsets.set(i++,*a++);
				post_update_offsets();
			}
			template<class T>
			void set_offsets(const std::vector<T>& v){
				for(unsigned int i=0;i<v.size();i++)
					m_offsets.set(i,v[i]);
				post_update_offsets();
			}
			void post_update_offsets(){
				m_dia2off.clear();
				for(unsigned int i = 0; i<m_offsets.size(); ++i)
					m_dia2off[m_offsets[i]] = i;
			}
			inline void set_offset(const index_type& idx, const index_type& val){
				m_offsets.set(idx,val);
				m_dia2off[val] = idx;
			}
			inline vec_type* get_dia(const int& i){ return new vec_type(m_stride,  m_vec->ptr() + m_dia2off[i] * m_stride, true); } ///< Return vector containing specified diagonal.
			inline const intvec_type& get_offsets()const{return m_offsets;}
			inline       intvec_type& get_offsets()     {return m_offsets;}
			inline int get_offset(const index_type& idx)const ///< Return offset of specified diagonal
			{
				return m_offsets[idx];
			}

			// ******************************
			// read access
			// ******************************
			value_type operator()(const index_type& i, const index_type& j)const ///< Return entry (i,j)
			{
				int off = (int)j - (int)i/m_row_fact;
				typename std::map<int,index_type>::const_iterator it = m_dia2off.find(off);
				if( it == m_dia2off.end() )
					return (value_type) 0;
				return (*m_vec)[ it->second * m_stride +i  ];
			}
	};
}

#endif /* __SPARSE_MATRIX_HPP__ */

