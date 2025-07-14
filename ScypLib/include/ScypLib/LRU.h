#pragma once
#include<list>
#include<unordered_map>
#include<cassert>

namespace sl
{
	template<typename T>
	class LRU
	{
	public:
		LRU() = default;
		void Push(T key)
		{
			Erase(key);
			accessOrder.push_back(key);
			keyPos[key] = std::prev(accessOrder.end());
		}
		void PopLRU()
		{
			assert(!accessOrder.empty());
			keyPos.erase(GetLRU());
			accessOrder.pop_front();
		}
		void Trim(size_t size)
		{
			while (accessOrder.size() > size)
			{
				PopLRU();
			}
		}
		void Erase(T key)
		{
			if (keyPos.contains(key))
			{
				accessOrder.erase(keyPos[key]);
				keyPos.erase(key);
			}
		}
		T GetLRU() const
		{
			assert(!accessOrder.empty());
			return accessOrder.front();
		}
	private:
		std::list<T> accessOrder;
		std::unordered_map<T, typename std::list<T>::iterator> keyPos;
	};
}