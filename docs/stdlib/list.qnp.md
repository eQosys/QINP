
# Stdlib - stdlib/list.qnp

## Overview
 - [Functions](#functions)
 - [Packs/Unions](#packs-unions)
 - [Details](#details)


## Functions
 - [fn<std.List.Node*> std.List.append(std.List.Node* list, std.List.Node* node)](#ref_25fcc4b61b9b3de1772b55d81f1fa32e)
 - [fn<std.List.Node*> std.List.append(std.List.Node* list, void* data)](#ref_296cd905ff75c18fe0c33cf214691b84)
 - [fn<std.List.Node*> std.List.create(void* data)](#ref_b2a329fecf413623aa381c00fc8cbd76)
 - [fn<> std.List.destroy(std.List.Node* node)](#ref_3d50fd068dc4f5f0e09a291f3e1ce691)
 - [fn<std.List.Node*> std.List.insert(std.List.Node* list, std.List.Node* node)](#ref_09e2ac15eb4f4bd4f14ec4978c574ee1)
 - [fn<std.List.Node*> std.List.insert(std.List.Node* node, void* data)](#ref_46f53c89b5cebf63772a5c3cdb1bacc9)
 - [fn<std.List.Node*> std.List.last(std.List.Node* list)](#ref_fa91d57e5c66f80cd6be0d370764048e)
 - [fn<u64> std.List.length(std.List.Node const* list)](#ref_d7fa25e1bc02f6978c06432879ebb776)
 - [fn<std.List.Node*> std.List.remove(std.List.Node* list, std.List.Node* node)](#ref_4671747c320b75a8e99e64113946830d)

## Packs/Unions
 - pack std.List.Node

## Details
#### <a id="ref_25fcc4b61b9b3de1772b55d81f1fa32e"/>fn<std.List.Node*> std.List.append(std.List.Node* list, std.List.Node* node)
> [Declaration](/stdlib/list.qnp?plain=1#L32) | [Definition](/stdlib/list.qnp?plain=1#L79)
```qinp
\\ Appends the given node to the list.
\\ @param list The list to append to.
\\ @param node The node to append.
\\ @return The appended node.
```
#### <a id="ref_296cd905ff75c18fe0c33cf214691b84"/>fn<std.List.Node*> std.List.append(std.List.Node* list, void* data)
> [Declaration](/stdlib/list.qnp?plain=1#L26) | [Definition](/stdlib/list.qnp?plain=1#L76)
```qinp
\\ Creates a new node with the given data and append it to the list.
\\ @param list The list to append to. If null, a new list will be created.
\\ @param data The data to append.
\\ @return The new node.
```
#### <a id="ref_b2a329fecf413623aa381c00fc8cbd76"/>fn<std.List.Node*> std.List.create(void* data)
> [Declaration](/stdlib/list.qnp?plain=1#L16) | [Definition](/stdlib/list.qnp?plain=1#L67)
```qinp
\\ Creates a new node with the given data.
\\ @param data The data to store in the node.
\\ @return The new node.
```
#### <a id="ref_3d50fd068dc4f5f0e09a291f3e1ce691"/>fn<> std.List.destroy(std.List.Node* node)
> [Declaration](/stdlib/list.qnp?plain=1#L20) | [Definition](/stdlib/list.qnp?plain=1#L73)
```qinp
\\ Destroys the given node.
\\ @param node The node to destroy.
```
#### <a id="ref_09e2ac15eb4f4bd4f14ec4978c574ee1"/>fn<std.List.Node*> std.List.insert(std.List.Node* list, std.List.Node* node)
> [Declaration](/stdlib/list.qnp?plain=1#L44) | [Definition](/stdlib/list.qnp?plain=1#L89)
```qinp
\\ Inserts the given node (of a list) after the given node of a list.
\\ @param list The node to insert after.
\\ @param node The node to insert.
\\ @return The inserted node.
```
#### <a id="ref_46f53c89b5cebf63772a5c3cdb1bacc9"/>fn<std.List.Node*> std.List.insert(std.List.Node* node, void* data)
> [Declaration](/stdlib/list.qnp?plain=1#L38) | [Definition](/stdlib/list.qnp?plain=1#L86)
```qinp
\\ Inserts a new node with the given data after the given node.
\\ @param node The node to insert after. If null, a new list will be created.
\\ @param data The data to insert.
\\ @return The new node.
```
#### <a id="ref_fa91d57e5c66f80cd6be0d370764048e"/>fn<std.List.Node*> std.List.last(std.List.Node* list)
> [Declaration](/stdlib/list.qnp?plain=1#L60) | [Definition](/stdlib/list.qnp?plain=1#L124)
```qinp
\\ Returns the last node of the given list.
\\ @param list The list to get the last node of.
\\ @return The last node of the list.
```
#### <a id="ref_d7fa25e1bc02f6978c06432879ebb776"/>fn<u64> std.List.length(std.List.Node const* list)
> [Declaration](/stdlib/list.qnp?plain=1#L55) | [Definition](/stdlib/list.qnp?plain=1#L117)
```qinp
\\ Returns the length of the given list.
\\ @param list The list to get the length of.
\\ @return The length of the list.
```
#### <a id="ref_4671747c320b75a8e99e64113946830d"/>fn<std.List.Node*> std.List.remove(std.List.Node* list, std.List.Node* node)
> [Declaration](/stdlib/list.qnp?plain=1#L50) | [Definition](/stdlib/list.qnp?plain=1#L97)
```qinp
\\ Removes and destroys the given node from the list.
\\ @param list The list to remove from.
\\ @param node The node to remove.
\\ @return The node after the removed node. If the removed node was the last node, null is returned.
```

