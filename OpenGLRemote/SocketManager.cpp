﻿#include "SocketManager.h"

enum : int {
	TRANSLATION = 0,
	ROTATION = 1,
	SCALE = 2,
	CLOSE = 3,
	TRANSLATION_DRAG = 4,
	ROTATION_DRAG = 5,
	TEXTURE_PAINTING = 6,
	BRUSH_COLOR_R = 7,
	BRUSH_COLOR_G = 8,
	BRUSH_COLOR_B = 9,
	BRUSH_OPACITY = 10,
	BRUSH_STIFFNESS = 11,
	BRUSH_SIZE = 12
};

std::vector<std::string> SocketManager::split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

void SocketManager::StartServer() {
	//WebSocket (WS)-server at port 8080 using 4 threads
	WsServer server(8080, 2);

	auto& echo = server.endpoint["^/echo/?$"];

	echo.onmessage = [&server, this](shared_ptr<WsServer::Connection> connection, shared_ptr<WsServer::Message> message) {
		auto message_str = message->string();
		string msg = message_str;
		std::vector<string> recievedElements;
		std::vector<string> vectorElements;
		float s1, s2, d;

		recievedElements = split(msg, ';');

		if (recievedElements.size() == 2)
		{
			std::pair<int, string> recievedValues(boost::lexical_cast<int>(recievedElements.at(0)), boost::lexical_cast<string>(recievedElements.at(1)));
			switch (recievedValues.first) {
			case TRANSLATION: 
				vectorElements = split(recievedValues.second, ',');
				data->acceleration.x = std::stof(vectorElements[0]);
				data->acceleration.y = std::stof(vectorElements[1]);
				data->acceleration.z = std::stof(vectorElements[2]);
				break;

			case ROTATION: 
				vectorElements = split(recievedValues.second, ',');
				data->rotation.x = std::stof(vectorElements[0]);
				data->rotation.y = std::stof(vectorElements[1]);
				data->rotation.z = std::stof(vectorElements[2]);
				break;

			case SCALE: 
				vectorElements = split(recievedValues.second, ',');
				s1 = std::stof(vectorElements[0]);
				s2 = std::stof(vectorElements[1]);
				d = - s1 - s2;
				data->scale.x = d;
				data->scale.y = d;
				data->scale.z = d;
				break;

			case CLOSE:
				FreeConsole(); 
				exit(0); // Exits OpenGL program
				break;

			case TRANSLATION_DRAG: 
				vectorElements = split(recievedValues.second, ',');
				data->drag.x = std::stof(vectorElements[0]);
				data->drag.y = std::stof(vectorElements[1]);
				break;

			case ROTATION_DRAG: // rotation drag
				vectorElements = split(recievedValues.second, ',');
				data->rot_drag.x = std::stof(vectorElements[0]);
				data->rot_drag.y = std::stof(vectorElements[1]);
				break;
			case TEXTURE_PAINTING: // texture painting
				std::cout << "tex_painting\n";
				if (data->texture_painting != 1.0f) {
					data->texture_painting = 1.0f;
				}
				else {
					data->texture_painting = 0.0f;
				}
				break;
			case BRUSH_COLOR_R: 
				data->painting_color.x = std::stof(recievedValues.second);
				break;
			case BRUSH_COLOR_G: 
				data->painting_color.y = std::stof(recievedValues.second);
				break;
			case BRUSH_COLOR_B:
				data->painting_color.z = std::stof(recievedValues.second);
				break;
			case BRUSH_OPACITY:
				data->opacity = std::stof(recievedValues.second);
				break;
			case BRUSH_STIFFNESS:
				data->brush_stiffness = std::stof(recievedValues.second);
				break;
			case BRUSH_SIZE: 
				data->brush_size = data->biggest_brush_size * std::stof(recievedValues.second);
				break;
			}



			auto send_stream = make_shared<WsServer::SendStream>();
			*send_stream << msg;
			//server.send is an asynchronous function
			server.send(connection, send_stream, [](const boost::system::error_code& ec) { 
				if (ec) {
					cout << "Server: Error sending message. " <<
						//See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
						"Error: " << ec << ", error message: " << ec.message() << endl;
				}
			});

		}

	};

	echo.onopen = [&server](shared_ptr<WsServer::Connection> connection) {
		cout << "Server: Opened connection " << (size_t)connection.get() << endl;

		for (int i = 0; i < 12; i++) {
			auto send_stream = make_shared<WsServer::SendStream>();
			*send_stream << i << ";0,0,0";
			//server.send is an asynchronous function
			server.send(connection, send_stream, [](const boost::system::error_code& ec) {
				if (ec) {
					cout << "Server: Error sending message. " <<
						//See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
						"Error: " << ec << ", error message: " << ec.message() << endl;
				}
			});
		}

	};

	//See RFC 6455 7.4.1. for status codes
	echo.onclose = [](shared_ptr<WsServer::Connection> connection, int status, const string& reason) {
		cout << "Server: Closed connection " << (size_t)connection.get() << " with status code " << status << endl;
	};

	//See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
	echo.onerror = [](shared_ptr<WsServer::Connection> connection, const boost::system::error_code& ec) {
		cout << "Server: Error in connection " << (size_t)connection.get() << ". " <<
			"Error: " << ec << ", error message: " << ec.message() << endl;
	};

	thread server_thread([&server]() {
		//Start WS-server
		server.start();
	});

	//Wait for server to start so that the client can connect
	this_thread::sleep_for(chrono::seconds(5));

	server_thread.join();

}