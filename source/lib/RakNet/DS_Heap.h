/// \file DS_Heap.h
/// \internal
/// \brief Heap (Also serves as a priority queue)
///
/// This file is part of RakNet Copyright 2003 Jenkins Software LLC
///
/// Usage of RakNet is subject to the appropriate license agreement.


#ifndef __RAKNET_HEAP_H
#define __RAKNET_HEAP_H

#include "RakMemoryOverride.h"
#include "DS_List.h"
#include "Export.h"
#include "RakAssert.h"

#ifdef _MSC_VER
#pragma warning( push )
#endif

/// The namespace DataStructures was only added to avoid compiler errors for commonly named data structures
/// As these data structures are stand-alone, you can use them outside of RakNet for your own projects if you wish.
namespace DataStructures
{
	template <class weight_type, class data_type, bool isMaxHeap>
	class RAK_DLL_EXPORT Heap
	{
	public:
		struct HeapNode
		{
			HeapNode() {}
			HeapNode(const weight_type &w, const data_type &d) : weight(w), data(d) {}
			weight_type weight; // I'm assuming key is a native numerical type - float or int
			data_type data;
		};

		Heap();
		~Heap();
		void Push(const weight_type &weight, const data_type &data, const char *file, unsigned int line);
		/// Call before calling PushSeries, for a new series of items
		void StartSeries(void) {optimizeNextSeriesPush=false;}
		/// If you are going to push a list of items, where the weights of the items on the list are in order and follow the heap order, PushSeries is faster than Push()
		void PushSeries(const weight_type &weight, const data_type &data, const char *file, unsigned int line);
		data_type Pop(const unsigned startingIndex);
		data_type Peek(const unsigned startingIndex=0) const;
		weight_type PeekWeight(const unsigned startingIndex=0) const;
		void Clear(bool doNotDeallocateSmallBlocks, const char *file, unsigned int line);
		data_type& operator[] ( const unsigned int position ) const;
		unsigned Size(void) const;

	protected:
		unsigned LeftChild(const unsigned i) const;
		unsigned RightChild(const unsigned i) const;
		unsigned Parent(const unsigned i) const;
		void Swap(const unsigned i, const unsigned j);
		DataStructures::List<HeapNode> heap;
		bool optimizeNextSeriesPush;
	};

	template  <class weight_type, class data_type, bool isMaxHeap>
		Heap<weight_type, data_type, isMaxHeap>::Heap()
	{
		optimizeNextSeriesPush=false;
	}

	template  <class weight_type, class data_type, bool isMaxHeap>
		Heap<weight_type, data_type, isMaxHeap>::~Heap()
	{
		//Clear(true, _FILE_AND_LINE_);
	}

	template  <class weight_type, class data_type, bool isMaxHeap>
	void Heap<weight_type, data_type, isMaxHeap>::PushSeries(const weight_type &weight, const data_type &data, const char *file, unsigned int line)
	{
		if (optimizeNextSeriesPush==false)
		{
			// If the weight of what we are inserting is greater than / less than in order of the heap of every sibling and sibling of parent, then can optimize next push
			unsigned currentIndex = heap.Size();
			unsigned parentIndex;
			if (currentIndex>0)
			{
				for (parentIndex = Parent(currentIndex); parentIndex < currentIndex; parentIndex++)
				{
#ifdef _MSC_VER
#pragma warning(disable:4127)   // conditional expression is constant
#endif
					if (isMaxHeap)
					{
						// Every child is less than its parent
						if (weight>heap[parentIndex].weight)
						{
							// Can't optimize
							Push(weight,data,file,line);
							return;
						}
					}
					else
					{
						// Every child is greater than than its parent
						if (weight<heap[parentIndex].weight)
						{
							// Can't optimize
							Push(weight,data,file,line);
							return;
						}
					}
				}
			}

			// Parent's subsequent siblings and this row's siblings all are less than / greater than inserted element. Can insert all further elements straight to the end
			heap.Insert(HeapNode(weight, data), file, line);
			optimizeNextSeriesPush=true;
		}
		else
		{
			heap.Insert(HeapNode(weight, data), file, line);
		}
	}

	template  <class weight_type, class data_type, bool isMaxHeap>
	void Heap<weight_type, data_type, isMaxHeap>::Push(const weight_type &weight, const data_type &data, const char *file, unsigned int line)
	{
		unsigned currentIndex = heap.Size();
		unsigned parentIndex;
		heap.Insert(HeapNode(weight, data), file, line);
		while (currentIndex!=0)
		{
			parentIndex = Parent(currentIndex);
#ifdef _MSC_VER
#pragma warning( disable : 4127 ) // warning C4127: conditional expression is constant
#endif
			if (isMaxHeap)
			{
				if (heap[parentIndex].weight < weight)
				{
					Swap(currentIndex, parentIndex);
					currentIndex=parentIndex;
				}
				else
					break;
			}
			else
			{
				if (heap[parentIndex].weight > weight)
				{
					Swap(currentIndex, parentIndex);
					currentIndex=parentIndex;
				}
				else
					break;
			}
		}
	}

	template  <class weight_type, class data_type, bool isMaxHeap>
	data_type Heap<weight_type, data_type, isMaxHeap>::Pop(const unsigned startingIndex)
	{
		// While we have children, swap out with the larger of the two children.

		// This line will assert on an empty heap
		data_type returnValue=heap[startingIndex].data;

		// Move the last element to the head, and re-heapify
		heap[startingIndex]=heap[heap.Size()-1];

		unsigned currentIndex,leftChild,rightChild;
		weight_type currentWeight;
		currentIndex=startingIndex;
		currentWeight=heap[startingIndex].weight;
		heap.RemoveFromEnd();

#ifdef _MSC_VER
#pragma warning( disable : 4127 ) // warning C4127: conditional expression is constant
#endif
		while (1)
		{
			leftChild=LeftChild(currentIndex);
			rightChild=RightChild(currentIndex);
			if (leftChild >= heap.Size())
			{
				// Done
				return returnValue;
			}
			if (rightChild >= heap.Size())
			{
				// Only left node.
				if ((isMaxHeap==true && currentWeight < heap[leftChild].weight) ||
					(isMaxHeap==false && currentWeight > heap[leftChild].weight))
						Swap(leftChild, currentIndex);

				return returnValue;
			}
			else
			{
				// Swap with the bigger/smaller of the two children and continue
				if (isMaxHeap)
				{
					if (heap[leftChild].weight <= currentWeight && heap[rightChild].weight <= currentWeight)
						return returnValue;

					if (heap[leftChild].weight > heap[rightChild].weight)
					{
						Swap(leftChild, currentIndex);
						currentIndex=leftChild;
					}
					else
					{
						Swap(rightChild, currentIndex);
						currentIndex=rightChild;
					}
				}
				else
				{
					if (heap[leftChild].weight >= currentWeight && heap[rightChild].weight >= currentWeight)
						return returnValue;

					if (heap[leftChild].weight < heap[rightChild].weight)
					{
						Swap(leftChild, currentIndex);
						currentIndex=leftChild;
					}
					else
					{
						Swap(rightChild, currentIndex);
						currentIndex=rightChild;
					}
				}
			}
		}
	}

	template  <class weight_type, class data_type, bool isMaxHeap>
	inline data_type Heap<weight_type, data_type, isMaxHeap>::Peek(const unsigned startingIndex) const
	{
		return heap[startingIndex].data;
	}

	template  <class weight_type, class data_type, bool isMaxHeap>
	inline weight_type Heap<weight_type, data_type, isMaxHeap>::PeekWeight(const unsigned startingIndex) const
	{
		return heap[startingIndex].weight;
	}

	template  <class weight_type, class data_type, bool isMaxHeap>
		void Heap<weight_type, data_type, isMaxHeap>::Clear(bool doNotDeallocateSmallBlocks, const char *file, unsigned int line)
	{
		heap.Clear(doNotDeallocateSmallBlocks, file, line);
	}

	template <class weight_type, class data_type, bool isMaxHeap>
	inline data_type& Heap<weight_type, data_type, isMaxHeap>::operator[] ( const unsigned int position ) const
	{
		return heap[position].data;
	}
	template <class weight_type, class data_type, bool isMaxHeap>
		unsigned Heap<weight_type, data_type, isMaxHeap>::Size(void) const
	{
		return heap.Size();
	}

	template <class weight_type, class data_type, bool isMaxHeap>
	inline unsigned Heap<weight_type, data_type, isMaxHeap>::LeftChild(const unsigned i) const
	{
		return i*2+1;
	}

	template <class weight_type, class data_type, bool isMaxHeap>
	inline unsigned Heap<weight_type, data_type, isMaxHeap>::RightChild(const unsigned i) const
	{
		return i*2+2;
	}

	template <class weight_type, class data_type, bool isMaxHeap>
	inline unsigned Heap<weight_type, data_type, isMaxHeap>::Parent(const unsigned i) const
	{
#ifdef _DEBUG
		RakAssert(i!=0);
#endif
		return (i-1)/2;
	}

	template <class weight_type, class data_type, bool isMaxHeap>
	void Heap<weight_type, data_type, isMaxHeap>::Swap(const unsigned i, const unsigned j)
	{
		HeapNode temp;
		temp=heap[i];
		heap[i]=heap[j];
		heap[j]=temp;
	}
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif
