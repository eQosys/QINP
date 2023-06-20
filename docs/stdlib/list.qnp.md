
# Stdlib - stdlib/list.qnp

## Overview
 - [Functions](#functions)
 - [Packs/Unions](#packs-unions)
 - [Details](#details)


## Functions
 - [fn\<std.List.Node*\> std.List.append(std.List.Node* list, std.List.Node* node)](#ref_3a6d9635c886314aad8acd25e2874d6a)
 - [fn\<std.List.Node*\> std.List.append(std.List.Node* list, void* data)](#ref_299bb8a31e3b98396305c7457917e10e)
 - [fn\<std.List.Node*\> std.List.create(void* data)](#ref_942c9fa3b801ad8f890782a69d488b61)
 - [fn\<\> std.List.destroy(std.List.Node* node)](#ref_af92380675a05cfc3800d8e0442612d2)
 - [fn\<std.List.Node*\> std.List.insert(std.List.Node* list, std.List.Node* node)](#ref_23ef3ffd16b491ac646fbf08c2149124)
 - [fn\<std.List.Node*\> std.List.insert(std.List.Node* node, void* data)](#ref_5835636c0b27f240fc20fbcb7f5d540d)
 - [fn\<std.List.Node*\> std.List.last(std.List.Node* list)](#ref_e542970037094ea68cd3073bcd70a8e1)
 - [fn\<u64\> std.List.length(std.List.Node const* list)](#ref_471f5bf22d9fa6bf2b71b1f4c3c03a3f)
 - [fn\<std.List.Node*\> std.List.remove(std.List.Node* list, std.List.Node* node)](#ref_319f469b689e913cd3fd5191616b5665)

## Packs/Unions
 - pack std.List.Node

## Details
#### <a id="ref_3a6d9635c886314aad8acd25e2874d6a"/>fn\<std.List.Node*\> std.List.append(std.List.Node* list, std.List.Node* node)
> [Declaration](/stdlib/list.qnp?plain=1#L33) | [Definition](/stdlib/list.qnp?plain=1#L84)
```qinp
Appends the given node to the list.
@param list The list to append to.
@param node The node to append.
@return The appended node.
```
#### <a id="ref_299bb8a31e3b98396305c7457917e10e"/>fn\<std.List.Node*\> std.List.append(std.List.Node* list, void* data)
> [Declaration](/stdlib/list.qnp?plain=1#L27) | [Definition](/stdlib/list.qnp?plain=1#L81)
```qinp
Creates a new node with the given data and append it to the list.
@param list The list to append to. If null, a new list will be created.
@param data The data to append.
@return The new node.
```
#### <a id="ref_942c9fa3b801ad8f890782a69d488b61"/>fn\<std.List.Node*\> std.List.create(void* data)
> [Declaration](/stdlib/list.qnp?plain=1#L17) | [Definition](/stdlib/list.qnp?plain=1#L68)
```qinp
Creates a new node with the given data.
@param data The data to store in the node.
@return The new node.
```
#### <a id="ref_af92380675a05cfc3800d8e0442612d2"/>fn\<\> std.List.destroy(std.List.Node* node)
> [Declaration](/stdlib/list.qnp?plain=1#L21) | [Definition](/stdlib/list.qnp?plain=1#L74)
```qinp
Destroys the given node.
@param node The node to destroy.
```
#### <a id="ref_23ef3ffd16b491ac646fbf08c2149124"/>fn\<std.List.Node*\> std.List.insert(std.List.Node* list, std.List.Node* node)
> [Declaration](/stdlib/list.qnp?plain=1#L45) | [Definition](/stdlib/list.qnp?plain=1#L94)
```qinp
Inserts the given node (of a list) after the given node of a list.
@param list The node to insert after.
@param node The node to insert.
@return The inserted node.
```
#### <a id="ref_5835636c0b27f240fc20fbcb7f5d540d"/>fn\<std.List.Node*\> std.List.insert(std.List.Node* node, void* data)
> [Declaration](/stdlib/list.qnp?plain=1#L39) | [Definition](/stdlib/list.qnp?plain=1#L91)
```qinp
Inserts a new node with the given data after the given node.
@param node The node to insert after. If null, a new list will be created.
@param data The data to insert.
@return The new node.
```
#### <a id="ref_e542970037094ea68cd3073bcd70a8e1"/>fn\<std.List.Node*\> std.List.last(std.List.Node* list)
> [Declaration](/stdlib/list.qnp?plain=1#L61) | [Definition](/stdlib/list.qnp?plain=1#L129)
```qinp
Returns the last node of the given list.
@param list The list to get the last node of.
@return The last node of the list.
```
#### <a id="ref_471f5bf22d9fa6bf2b71b1f4c3c03a3f"/>fn\<u64\> std.List.length(std.List.Node const* list)
> [Declaration](/stdlib/list.qnp?plain=1#L56) | [Definition](/stdlib/list.qnp?plain=1#L122)
```qinp
Returns the length of the given list.
@param list The list to get the length of.
@return The length of the list.
```
#### <a id="ref_319f469b689e913cd3fd5191616b5665"/>fn\<std.List.Node*\> std.List.remove(std.List.Node* list, std.List.Node* node)
> [Declaration](/stdlib/list.qnp?plain=1#L51) | [Definition](/stdlib/list.qnp?plain=1#L102)
```qinp
Removes and destroys the given node from the list.
@param list The list to remove from.
@param node The node to remove.
@return The node after the removed node. If the removed node was the last node, null is returned.
```

