#pragma once

namespace ciallo
{
	/**
	 * Main registry is used for storing scene data and runtime data.
	 * Redo undo registry is used for storing components need to be retraced back in redo undo operation.
	 */
	class Project
	{
		entt::registry m_registry;
		entt::registry m_redoUndoRegistry;
	public:
		Project() = default;
		Project(const Project& other) = delete;
		Project(Project&& other) noexcept = default;
		Project& operator=(const Project& other) = delete;
		Project& operator=(Project&& other) noexcept = default;
		~Project() = default;
	
		entt::registry& registry()
		{
			return m_registry;
		}

		entt::registry& redoUndoRegistry()
		{
			return m_redoUndoRegistry;
		}
	};
}
