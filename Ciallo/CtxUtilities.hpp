#pragma once

// Miscellaneous objects delivered along with ctx,
namespace ciallo
{
	class CommandBuffers
	{
		vk::CommandBuffer m_main;

	public:
		vk::CommandBuffer mainCb() const
		{
			return m_main;
		}

		void setMain(vk::CommandBuffer cb)
		{
			m_main = cb;
		}
	};

	class Cleaner
	{
		std::vector<uint32_t> m_componentIDs;
	public:
		template <typename Cpo>
		void cleanEndFrame()
		{
			m_componentIDs.push_back(entt::type_hash<Cpo>());
		}

		void cleanup(entt::registry& registry)
		{
			for (auto id : m_componentIDs)
			{
				registry.storage(id)->second.clear();
			}
			m_componentIDs.clear();
		}
	};
}
