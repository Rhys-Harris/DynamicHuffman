# Dynamic Huffman
This project takes Huffman encoding and attempts to make it more efficient.
It does this by allowing dynamic grouping sizes, cleaning up the Huffman tree.

For example, here is some text to compress:
```
AANDSABSDNDODOOOOHKDFJDA
```

Huffman would beneficially compress the A and D characters to very few bits.
However, notice that every occurence of 'F' (1) is followed by a 'J'.
In this way, these characters would be groups together as a single node.
This shrinks the huffman tree, meaning smaller compressed size.

## Drawbacks
First, now each entry in the table has a dynamic size.
This makes it more complicated to store this information, and read it back.

Second, it increases the size of the table.
The table now needs to store potentially multiple characters for each entry.
The amount of characters stored also needs to be stored.

## Metadata
The original Huffman metadata I stored was like this.
This is from my [previous project](https://github.com/Rhys-Harris/Huffman).

```
4 bytes: How many entries are in the table
4 bytes: The last byte index of the compressed text
4 bytes: the last bit index of the compressed text

= TOTAL: 12 bytes =
```

This creates a 2^32 limit to how many nodes can be in the huffman tree.
We also have a 2^32 (~4gb) compressed file limit.

We don't actually need 4 bytes for the last bit index, since:
`0 <= last bit index <= 7`
So we can then put this into a single bit instead.

It would also be helpful to store the length of the original file was kept.
Therefore, our new metadata is this.

```
4 bytes: How many entries are in the table
4 bytes: Original file size
4 bytes: The last byte index of the compressed text
1 bytes: the last bit index of the compressed text

= TOTAL: 13 bytes =
```

## Table
The original Huffman table I stored was like this (example is a single entry).

```
4 bytes: Index of this node's parent
1 bytes: This entry's symbol (char)
1 bytes: Whether this is a left or right node from its parent

= TOTAL: 6 bytes (multiply by N nodes) =
```

Now we need to have a dynamically long symbol, and also keep the symbol's length.

```
4 bytes: Index of this node's parent
1 bytes: Number of symbols this entry covers
1 bytes: Whether this is a left or right node from its parent
1-255 bytes: This entry's symbol (char)

= TOTAL: 7-261 bytes (multiply by N nodes) =
```

Here we've placed a limit that an entry can only cover 255 characters.
This is fine for our use case, but could easily be changed.

We only store the necessary number of chars on storage.
However, in memory we can use a static array of length 255.
The cost for this is manageable, depending on the memory limit of the computer.
