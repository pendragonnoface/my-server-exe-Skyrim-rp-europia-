#pragma once
#include "IDRC_API.h"

namespace Messaging
{
	using InterfaceVersion1 = ::IDRC_API::IVIDRC1;

	class IDRCInterface : public InterfaceVersion1
	{
	private:
		IDRCInterface() noexcept;
		virtual ~IDRCInterface() noexcept;

	public:
		static IDRCInterface* GetSingleton() noexcept
		{
			static IDRCInterface singleton;
			return std::addressof(singleton);
		}

		// InterfaceVersion1
        virtual unsigned long GetIDRCThreadId(void) const noexcept override;
		virtual RE::ActorHandle GetCurrentTarget() const noexcept override;
		virtual bool UseTarget() const noexcept override;
		virtual RE::ActorHandle GetDragon() const noexcept override;

	private:
		unsigned long apiTID = 0;
	};
}
