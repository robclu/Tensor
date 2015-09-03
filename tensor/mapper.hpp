// ----------------------------------------------------------------------------------------------------------
/// @file   Header file for mapper related classes and variables for mapping linear indices to multiple 
///         dimensions and from mapping multiple dimensions to a linear index etc ...
// ----------------------------------------------------------------------------------------------------------

/*
 * ----------------------------------------------------------------------------------------------------------
 *  Header file for mapper class
 *  Copyright (C) 2015 Rob Clucas robclu1818@gmail.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published
 *  by the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation,
 *  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * ----------------------------------------------------------------------------------------------------------
 */

#ifndef FTL_MAPPER_HPP
#define FTL_MAPPER_HPP

#include <nano/nano.hpp>

namespace ftl {
    
// Helper meta functions for the mapper namespace
namespace detail {
    
    template <typename Index, typename DimList, typename PrevDimSizes>
    struct offset_calculator;
   
    // Case for the very first iteration, no previous dimensinos
    template <typename Index, typename Head, typename... Tail>
    struct offset_calculator<Index, nano::list<Head, Tail...>, nano::empty_list>
    {
        static constexpr int offset = 
            Index::value                            +
            offset_calculator<Index                 ,   // Offset from the other dimensions 
                              nano::list<Tail...>   , 
                              nano::list<Head>          // Current dim size is passed as previoud dim size
                                  >::offset;
    };
           
    // Case for all cases but the first and last  
    template <typename Index, typename... DimList, typename... Passed>
    struct offset_calculator<Index, nano::list<DimList...>, nano::list<Passed...>>
    {
       static constexpr int offset =
           nano::multiplies<nano::list<DimList...>>::result;
    };
    
    // Base case
    template <typename Index, typename... Passed>
    struct offset_calculator<Index, nano::empty_list, nano::list<Passed...>>
    {
        
    };
    
    // ------------------------------------------------------------------------------------------------------
    /// @struct     index_calculator
    /// @brief      Takes an index of an element in contiguous memory, and using the mapping of that memory
    ///             space to dimensions, determines the index in each of the dimensions of the given index. 
    ///             For example, if a 3x3 matrix is stored column-major in memory, then index 4 in contiguous 
    ///             memory (0 indexing) maps to row 1 column 1. This metafunction, however, does the 
    ///             calculation for any dimensional structure at compile time.
    /// @tparam     Index           The index of the element in contiguous memory.
    /// @tparam     AlDimSizes      The sizes of all dimensions of the structure.
    /// @tparam     Iteration       The iteration of the calculation.
    /// @tparam     PrevDimSizes    The sizes of the dimensions used in previous iterations.
    /// @tparam     MappedIndices   The determined mapped indices.
    // ------------------------------------------------------------------------------------------------------
    template <typename  Index           				,
  			  typename  AllDImSizes						,
  			  int 		Iteration = 0   				,
              typename  PrevDimSizes = nano::list<>    	, 
              typename  MappedIndices = nano::list<>  	>
    struct index_calculator;
    
    // Recursive case
    template <typename      Index           , 
  			  typename 	    Head			,
  			  typename... 	Tail			,
  			  int 			Iteration		,
              typename...   PrevSizes       ,
              typename...   MappedIndices   >
    struct index_calculator<Index                       ,   
                		    nano::list<Head, Tail...>   ,
  						    Iteration 					,
                            nano::list<PrevSizes...>    ,   
                            nano::list<MappedIndices...>>
    {
        // Get the product of the dim sizes of the previous iterations
        // which is the offset in contiguous memory
        static constexpr int prev_product =
            nano::multiplies<nano::list<PrevSizes...>>::result;
        
        // The case for the first iteration is different, so check for that
        using new_index = typename std::conditional<
                                Iteration == 0                                                              , 
                                nano::int_t<Index::value % Head::value>                                     ,
                                nano::int_t<(Index::value % (prev_product * Head::value)) / prev_product>
                                    >::type;
        
        // Add the new index to the list of mapped indices and recurse, 
        // eliminating the dimension size that was just used
        using result = typename index_calculator<  
                                            Index                            		, 
                              				nano::list<Tail...>						,   
                              				Iteration + 1 							,
                                            nano::list<Head, PrevSizes...>   		,
                                            nano::list<MappedIndices..., new_index>	
                                                    >::result;
        
    };
    
    // Terminating case, just return the list of mapped indices
    template <typename Index, int Iteration, typename... PrevSizes, typename... MappedIndices>
    struct index_calculator< 
                            Index                       ,
      						nano::list<>				,
  							Iteration					,
                            nano::list<PrevSizes...>    , 
                            nano::list<MappedIndices...>>
    {
        using result = nano::list<MappedIndices...>;
    };

}       // End namespace detail

// ----------------------------------------------------------------------------------------------------------
/// @namespace  mapper
/// @brief      Mapping functions for converting an index in contiguous memory to its index values in the
///             multi-dimensional space which the contiguous memory represents (a matrix for example), and 
///             also for the reverse operations.
// ----------------------------------------------------------------------------------------------------------
namespace mapper {

template <typename Index, typename DimSizeList>
using index_to_dim_positions = typename detail::index_calculator<Index, DimSizeList>::result;

}           // End namespace mapper

}           // End namespace ftl

#endif      // FTL_MAPPER_HPP

