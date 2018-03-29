#pragma once

#include <NodeEditor.h>
#include <memory>
#include <vector>
#include <iostream>
#include <string>

namespace ed = ax::NodeEditor;

namespace rat
{
struct Node;

struct NodePin
{
	int Id;
	Node* Node = nullptr;
	std::string Name;
	ed::PinKind Kind;

	NodePin(int id, const std::string& name, ed::PinKind kind) :
		Id(id), Name(name), Kind(kind) { }
};

struct Node
{
	int Id;
	std::string Name;
	std::vector<std::unique_ptr<NodePin>> Inputs;
	std::vector<std::unique_ptr<NodePin>> Outputs;

	Node(int id, const std::string& name) :
		Id(id), Name(name) { }

	void createPin(const std::string& name, ed::PinKind pinKind);
};

struct NodeLink
{
	int Id;

	int StartPinId;
	int EndPinId;

	ImColor Color = { 255, 255, 255 };

	NodeLink(int id, int startPinId, int endPinId) :
		Id(id), StartPinId(startPinId), EndPinId(endPinId) { }
};

class NodeManager
{
private:
	std::vector<std::unique_ptr<Node>> _nodes;

	std::vector<std::unique_ptr<NodeLink>> _links;

	int _lastId = 0;

public:
	NodeManager() = default;
	~NodeManager() = default;

	NodePin* findPin(int pinId);

	Node* createNode(const std::string& name);

	void removeNode(int nodeId);

	auto& getNodes() { return _nodes; }

	NodeLink* createLink(int startPinId, int endPinId);

	bool isPinLinked(int pinId);

	void removeLink(int linkId);

	auto& getLinks() { return _links; }

private:
	int GetNewId() { return ++_lastId; }
};

}
