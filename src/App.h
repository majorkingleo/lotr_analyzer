#pragma once

#include <atomic>
#include <database.h>
#include <memory>
#include <functional>
#include <list>

struct App
{
	struct WaitForObject
	{
		std::atomic<bool> * m_running;
		
		WaitForObject( std::atomic<bool> & running )
		: m_running(&running)
		{
			*m_running = true;
		}

		WaitForObject( const WaitForObject & other ) = delete;

		WaitForObject( WaitForObject && other )
		: m_running( other.m_running )
		{
			
		}

		~WaitForObject()
		{
			if( m_running ) {
				*m_running = false;
				m_running = nullptr;
			}			
		}
	};

	std::atomic<bool> 					quit_request 		{false};
	
	Tools::ThreadedDatabase 			db {};
	std::function<void()> 				reconnect_db {};

private:
	std::mutex							m_wait_for_object_mutex;
	std::list<std::atomic<bool>>		m_wait_for_objects_list;

public:

	WaitForObject get_wait_for_object_handle() 
	{
		auto lock = std::scoped_lock( m_wait_for_object_mutex );
		return WaitForObject( m_wait_for_objects_list.emplace_back( true ) );
	}

	bool has_active_wait_for_objects() 
	{
		auto lock = std::scoped_lock( m_wait_for_object_mutex );
		for( const auto & obj : m_wait_for_objects_list ) {
			if( obj.load() ) {
				return true;
			}
		}
		return false;
	}
};

extern App APP;
