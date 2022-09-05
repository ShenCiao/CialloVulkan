#pragma once

/*
 * This class supposed to be able deal with relationship and undo/redo automatically. It not implemented yet.
 * In practice, use this class directly or create a component struct inherent from it.
 * 
 */ 
class EntityContainer
{
	std::vector<entt::entity> entities{};
public:
	int size() const
	{
		return static_cast<int>(entities.size());
	}

	void insert(int index, entt::entity e)
	{
		entities.insert(entities.begin()+index, e);
	}

	void push_back(entt::entity e)
	{
		entities.push_back(e);
	}

	void remove(int index)
	{
		entities.erase(entities.begin()+index);
	}

	int find(entt::entity e) const // return a index
	{
		auto i = std::ranges::find(entities, e);
		return static_cast<int>(i - entities.begin());
	}
};

struct ContainedBy
{
	entt::entity entity = entt::null;
};