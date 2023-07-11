
# Stdlib - stdlib/list.qnp

## Overview
 - [Functions](#functions)
 - [Packs/Unions](#packs-unions)
 - [Details](#details)


## Functions
 - [fn\<std.List_old.Node*\> std.List_old.append(std.List_old.Node* list, std.List_old.Node* node)](#ref_5f244a0710545960e0c9f4037d19dcf0)
 - [fn\<std.List_old.Node*\> std.List_old.append(std.List_old.Node* list, void* data)](#ref_549517eb95f189c71300dcaf2ee8eedb)
 - [fn\<std.List_old.Node*\> std.List_old.create(void* data)](#ref_fc869342d062f06088fb695d1ffd74b2)
 - [fn\<\> std.List_old.destroy(std.List_old.Node* node)](#ref_a01ebd05a019ccb18768b8b0552777ef)
 - [fn\<std.List_old.Node*\> std.List_old.insert(std.List_old.Node* list, std.List_old.Node* node)](#ref_18630a314087fcbb44ec0d9b07e48194)
 - [fn\<std.List_old.Node*\> std.List_old.insert(std.List_old.Node* node, void* data)](#ref_ef0bf0a0197c9d27d27b9a09ff18a23d)
 - [fn\<std.List_old.Node*\> std.List_old.last(std.List_old.Node* list)](#ref_2965f487acc2a9a946c5ce4ceb386254)
 - [fn\<u64\> std.List_old.length(std.List_old.Node const* list)](#ref_95764927dbd426cd36516e52027b0c15)
 - [fn\<std.List_old.Node*\> std.List_old.remove(std.List_old.Node* list, std.List_old.Node* node)](#ref_e94bc2d7ca52e877b5a9aa44ef6ccd09)

## Packs/Unions
 - pack std.List
 - [pack std.List_old.Node](#ref_b11e02dbd5165480da6f4a8f45c0d3c3)

## Details
#### <a id="ref_b11e02dbd5165480da6f4a8f45c0d3c3"/>pack std.List_old.Node
> [Definition](/stdlib/list.qnp?plain=1#L59)
```qinp
A single node in a list.
```
#### <a id="ref_5f244a0710545960e0c9f4037d19dcf0"/>fn\<std.List_old.Node*\> std.List_old.append(std.List_old.Node* list, std.List_old.Node* node)
> [Declaration](/stdlib/list.qnp?plain=1#L82) | [Definition](/stdlib/list.qnp?plain=1#L311)
```qinp
Appends the given node to the list.
@param list The list to append to.
@param node The node to append.
@return The appended node.
```
#### <a id="ref_549517eb95f189c71300dcaf2ee8eedb"/>fn\<std.List_old.Node*\> std.List_old.append(std.List_old.Node* list, void* data)
> [Declaration](/stdlib/list.qnp?plain=1#L76) | [Definition](/stdlib/list.qnp?plain=1#L308)
```qinp
Creates a new node with the given data and append it to the list.
@param list The list to append to. If null, a new list will be created.
@param data The data to append.
@return The new node.
```
#### <a id="ref_fc869342d062f06088fb695d1ffd74b2"/>fn\<std.List_old.Node*\> std.List_old.create(void* data)
> [Declaration](/stdlib/list.qnp?plain=1#L66) | [Definition](/stdlib/list.qnp?plain=1#L295)
```qinp
Creates a new node with the given data.
@param data The data to store in the node.
@return The new node.
```
#### <a id="ref_a01ebd05a019ccb18768b8b0552777ef"/>fn\<\> std.List_old.destroy(std.List_old.Node* node)
> [Declaration](/stdlib/list.qnp?plain=1#L70) | [Definition](/stdlib/list.qnp?plain=1#L301)
```qinp
Destroys the given node.
@param node The node to destroy.
```
#### <a id="ref_18630a314087fcbb44ec0d9b07e48194"/>fn\<std.List_old.Node*\> std.List_old.insert(std.List_old.Node* list, std.List_old.Node* node)
> [Declaration](/stdlib/list.qnp?plain=1#L94) | [Definition](/stdlib/list.qnp?plain=1#L321)
```qinp
Inserts the given node (of a list) after the given node of a list.
@param list The node to insert after.
@param node The node to insert.
@return The inserted node.
```
#### <a id="ref_ef0bf0a0197c9d27d27b9a09ff18a23d"/>fn\<std.List_old.Node*\> std.List_old.insert(std.List_old.Node* node, void* data)
> [Declaration](/stdlib/list.qnp?plain=1#L88) | [Definition](/stdlib/list.qnp?plain=1#L318)
```qinp
Inserts a new node with the given data after the given node.
@param node The node to insert after. If null, a new list will be created.
@param data The data to insert.
@return The new node.
```
#### <a id="ref_2965f487acc2a9a946c5ce4ceb386254"/>fn\<std.List_old.Node*\> std.List_old.last(std.List_old.Node* list)
> [Declaration](/stdlib/list.qnp?plain=1#L110) | [Definition](/stdlib/list.qnp?plain=1#L356)
```qinp
Returns the last node of the given list.
@param list The list to get the last node of.
@return The last node of the list.
```
#### <a id="ref_95764927dbd426cd36516e52027b0c15"/>fn\<u64\> std.List_old.length(std.List_old.Node const* list)
> [Declaration](/stdlib/list.qnp?plain=1#L105) | [Definition](/stdlib/list.qnp?plain=1#L349)
```qinp
Returns the length of the given list.
@param list The list to get the length of.
@return The length of the list.
```
#### <a id="ref_e94bc2d7ca52e877b5a9aa44ef6ccd09"/>fn\<std.List_old.Node*\> std.List_old.remove(std.List_old.Node* list, std.List_old.Node* node)
> [Declaration](/stdlib/list.qnp?plain=1#L100) | [Definition](/stdlib/list.qnp?plain=1#L329)
```qinp
Removes and destroys the given node from the list.
@param list The list to remove from.
@param node The node to remove.
@return The node after the removed node. If the removed node was the last node, null is returned.
```

