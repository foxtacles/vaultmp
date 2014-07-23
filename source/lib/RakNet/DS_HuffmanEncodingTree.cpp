/// \file
///
/// This file is part of RakNet Copyright 2003 Jenkins Software LLC
///
/// Usage of RakNet is subject to the appropriate license agreement.


#include "DS_HuffmanEncodingTree.h"
#include "DS_Queue.h"
#include "BitStream.h"
#include "RakAssert.h" 

#ifdef _MSC_VER
#pragma warning( push )
#endif

using namespace RakNet;

HuffmanEncodingTree::HuffmanEncodingTree()
{
	root = 0;
}

HuffmanEncodingTree::~HuffmanEncodingTree()
{
	FreeMemory();
}

void HuffmanEncodingTree::FreeMemory( void )
{
	if ( root == 0 )
		return ;

	// Use an in-order traversal to delete the tree
	DataStructures::Queue<HuffmanEncodingTreeNode *> nodeQueue;

	HuffmanEncodingTreeNode *node;

	nodeQueue.Push( root, _FILE_AND_LINE_  );

	while ( nodeQueue.Size() > 0 )
	{
		node = nodeQueue.Pop();

		if ( node->left )
			nodeQueue.Push( node->left, _FILE_AND_LINE_  );

		if ( node->right )
			nodeQueue.Push( node->right, _FILE_AND_LINE_  );

		RakNet::OP_DELETE(node, _FILE_AND_LINE_);
	}

	// Delete the encoding table
	for ( int i = 0; i < 256; i++ )
		rakFree_Ex(encodingTable[ i ].encoding, _FILE_AND_LINE_ );

	root = 0;
}


////#include <stdio.h>

// Given a frequency table of 256 elements, all with a frequency of 1 or more, generate the tree
void HuffmanEncodingTree::GenerateFromFrequencyTable( unsigned int frequencyTable[ 256 ] )
{
	int counter;
	HuffmanEncodingTreeNode * node;
	HuffmanEncodingTreeNode *leafList[ 256 ]; // Keep a copy of the pointers to all the leaves so we can generate the encryption table bottom-up, which is easier
	// 1.  Make 256 trees each with a weight equal to the frequency of the corresponding character
	DataStructures::LinkedList<HuffmanEncodingTreeNode *> huffmanEncodingTreeNodeList;

	FreeMemory();

	for ( counter = 0; counter < 256; counter++ )
	{
		node = RakNet::OP_NEW<HuffmanEncodingTreeNode>( _FILE_AND_LINE_ );
		node->left = 0;
		node->right = 0;
		node->value = (unsigned char) counter;
		node->weight = frequencyTable[ counter ];

		if ( node->weight == 0 )
			node->weight = 1; // 0 weights are illegal

		leafList[ counter ] = node; // Used later to generate the encryption table

		InsertNodeIntoSortedList( node, &huffmanEncodingTreeNodeList ); // Insert and maintain sort order.
	}


	// 2.  While there is more than one tree, take the two smallest trees and merge them so that the two trees are the left and right
	// children of a new node, where the new node has the weight the sum of the weight of the left and right child nodes.
#ifdef _MSC_VER
#pragma warning( disable : 4127 ) // warning C4127: conditional expression is constant
#endif
	while ( 1 )
	{
		huffmanEncodingTreeNodeList.Beginning();
		HuffmanEncodingTreeNode *lesser, *greater;
		lesser = huffmanEncodingTreeNodeList.Pop();
		greater = huffmanEncodingTreeNodeList.Pop();
		node = RakNet::OP_NEW<HuffmanEncodingTreeNode>( _FILE_AND_LINE_ );
		node->left = lesser;
		node->right = greater;
		node->weight = lesser->weight + greater->weight;
		lesser->parent = node;  // This is done to make generating the encryption table easier
		greater->parent = node;  // This is done to make generating the encryption table easier

		if ( huffmanEncodingTreeNodeList.Size() == 0 )
		{
			// 3. Assign the one remaining node in the list to the root node.
			root = node;
			root->parent = 0;
			break;
		}

		// Put the new node back into the list at the correct spot to maintain the sort.  Linear search time
		InsertNodeIntoSortedList( node, &huffmanEncodingTreeNodeList );
	}

	bool tempPath[ 256 ]; // Maximum path length is 256
	unsigned short tempPathLength;
	HuffmanEncodingTreeNode *currentNode;
	RakNet::BitStream bitStream;

	// Generate the encryption table. From before, we have an array of pointers to all the leaves which contain pointers to their parents.
	// This can be done more efficiently but this isn't bad and it's way easier to program and debug

	for ( counter = 0; counter < 256; counter++ )
	{
		// Already done at the end of the loop and before it!
		tempPathLength = 0;

		// Set the current node at the leaf
		currentNode = leafList[ counter ];

		do
		{
			if ( currentNode->parent->left == currentNode )   // We're storing the paths in reverse order.since we are going from the leaf to the root
				tempPath[ tempPathLength++ ] = false;
			else
				tempPath[ tempPathLength++ ] = true;

			currentNode = currentNode->parent;
		}

		while ( currentNode != root );

		// Write to the bitstream in the reverse order that we stored the path, which gives us the correct order from the root to the leaf
		while ( tempPathLength-- > 0 )
		{
			if ( tempPath[ tempPathLength ] )   // Write 1's and 0's because writing a bool will write the BitStream TYPE_CHECKING validation bits if that is defined along with the actual data bit, which is not what we want
				bitStream.Write1();
			else
				bitStream.Write0();
		}

		// Read data from the bitstream, which is written to the encoding table in bits and bitlength. Note this function allocates the encodingTable[counter].encoding pointer
		encodingTable[ counter ].bitLength = ( unsigned char ) bitStream.CopyData( &encodingTable[ counter ].encoding );

		// Reset the bitstream for the next iteration
		bitStream.Reset();
	}
}

// Pass an array of bytes to array and a preallocated BitStream to receive the output
void HuffmanEncodingTree::EncodeArray( unsigned char *input, size_t sizeInBytes, RakNet::BitStream * output )
{		
	unsigned counter;

	// For each input byte, Write out the corresponding series of 1's and 0's that give the encoded representation
	for ( counter = 0; counter < sizeInBytes; counter++ )
	{
		output->WriteBits( encodingTable[ input[ counter ] ].encoding, encodingTable[ input[ counter ] ].bitLength, false ); // Data is left aligned
	}

	// Byte align the output so the unassigned remaining bits don't equate to some actual value
	if ( output->GetNumberOfBitsUsed() % 8 != 0 )
	{
		// Find an input that is longer than the remaining bits.  Write out part of it to pad the output to be byte aligned.
		unsigned char remainingBits = (unsigned char) ( 8 - ( output->GetNumberOfBitsUsed() % 8 ) );

		for ( counter = 0; counter < 256; counter++ )
			if ( encodingTable[ counter ].bitLength > remainingBits )
			{
				output->WriteBits( encodingTable[ counter ].encoding, remainingBits, false ); // Data is left aligned
				break;
			}

#ifdef _DEBUG
			RakAssert( counter != 256 );  // Given 256 elements, we should always be able to find an input that would be >= 7 bits

#endif

	}
}

unsigned HuffmanEncodingTree::DecodeArray( RakNet::BitStream * input, BitSize_t sizeInBits, size_t maxCharsToWrite, unsigned char *output )
{
	HuffmanEncodingTreeNode * currentNode;

	unsigned outputWriteIndex;
	outputWriteIndex = 0;
	currentNode = root;

	// For each bit, go left if it is a 0 and right if it is a 1.  When we reach a leaf, that gives us the desired value and we restart from the root

	for ( unsigned counter = 0; counter < sizeInBits; counter++ )
	{
		if ( input->ReadBit() == false )   // left!
			currentNode = currentNode->left;
		else
			currentNode = currentNode->right;

		if ( currentNode->left == 0 && currentNode->right == 0 )   // Leaf
		{

			if ( outputWriteIndex < maxCharsToWrite )
				output[ outputWriteIndex ] = currentNode->value;

			outputWriteIndex++;

			currentNode = root;
		}
	}

	return outputWriteIndex;
}

// Pass an array of encoded bytes to array and a preallocated BitStream to receive the output
void HuffmanEncodingTree::DecodeArray( unsigned char *input, BitSize_t sizeInBits, RakNet::BitStream * output )
{
	HuffmanEncodingTreeNode * currentNode;

	if ( sizeInBits <= 0 )
		return ;

	RakNet::BitStream bitStream( input, BITS_TO_BYTES(sizeInBits), false );

	currentNode = root;

	// For each bit, go left if it is a 0 and right if it is a 1.  When we reach a leaf, that gives us the desired value and we restart from the root
	for ( unsigned counter = 0; counter < sizeInBits; counter++ )
	{
		if ( bitStream.ReadBit() == false )   // left!
			currentNode = currentNode->left;
		else
			currentNode = currentNode->right;

		if ( currentNode->left == 0 && currentNode->right == 0 )   // Leaf
		{
			output->WriteBits( &( currentNode->value ), sizeof( char ) * 8, true ); // Use WriteBits instead of Write(char) because we want to avoid TYPE_CHECKING
			currentNode = root;
		}
	}
}

// Insertion sort.  Slow but easy to write in this case
void HuffmanEncodingTree::InsertNodeIntoSortedList( HuffmanEncodingTreeNode * node, DataStructures::LinkedList<HuffmanEncodingTreeNode *> *huffmanEncodingTreeNodeList ) const
{
	if ( huffmanEncodingTreeNodeList->Size() == 0 )
	{
		huffmanEncodingTreeNodeList->Insert( node );
		return ;
	}

	huffmanEncodingTreeNodeList->Beginning();

	unsigned counter = 0;
#ifdef _MSC_VER
#pragma warning( disable : 4127 ) // warning C4127: conditional expression is constant
#endif
	while ( 1 )
	{
		if ( huffmanEncodingTreeNodeList->Peek()->weight < node->weight )
			++( *huffmanEncodingTreeNodeList );
		else
		{
			huffmanEncodingTreeNodeList->Insert( node );
			break;
		}

		// Didn't find a spot in the middle - add to the end
		if ( ++counter == huffmanEncodingTreeNodeList->Size() )
		{
			huffmanEncodingTreeNodeList->End();

			huffmanEncodingTreeNodeList->Add( node )

				; // Add to the end
			break;
		}
	}
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif
