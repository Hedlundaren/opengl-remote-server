#include "SocketManager.h"


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
	WsServer server(8080, 4);

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
			case 1: // translation
				vectorElements = split(recievedValues.second, ',');
				data->acceleration.x = std::stof(vectorElements[0]);
				data->acceleration.y = std::stof(vectorElements[1]);
				data->acceleration.z = std::stof(vectorElements[2]);
				break;

			case 2: // rotation
				vectorElements = split(recievedValues.second, ',');
				data->rotation.x = std::stof(vectorElements[0]);
				data->rotation.y = std::stof(vectorElements[1]);
				data->rotation.z = std::stof(vectorElements[2]);
				break;

			case 3: // scale
				vectorElements = split(recievedValues.second, ',');
				s1 = std::stof(vectorElements[0]);
				s2 = std::stof(vectorElements[1]);
				d = s1 + s2;
				data->scale.x = d;
				data->scale.y = d;
				data->scale.z = d;
				break;

			case 4:
				FreeConsole(); // Closes console
				exit(0); // Exits OpenGL program
				break;

			case 5: // translate drag
				vectorElements = split(recievedValues.second, ',');
				data->drag.x = std::stof(vectorElements[0]);
				data->drag.y = std::stof(vectorElements[1]);
				break;

			case 6: // rotation drag
				vectorElements = split(recievedValues.second, ',');
				data->rot_drag.x = std::stof(vectorElements[0]);
				data->rot_drag.y = std::stof(vectorElements[1]);
				break;
			case 7: // rotation drag
				std::cout << "tex_painting\n";
				if (data->texture_painting != 1.0f) {
					data->texture_painting = 1.0f;
				}
				else {
					data->texture_painting = 0.0f;
				}
				break;
			case 1011: // color r
				data->painting_color.x = std::stof(recievedValues.second);
				break;
			case 1012: // color g
				data->painting_color.y = std::stof(recievedValues.second);
				break;
			case 1013: // color b
				data->painting_color.z = std::stof(recievedValues.second);
				break;
			case 102: // opacity
				data->opacity = std::stof(recievedValues.second);
				
				break;
			case 103: // stiffness
				data->brush_stiffness = std::stof(recievedValues.second);
				break;
			case 104: // size
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
		auto send_stream = make_shared<WsServer::SendStream>();
		*send_stream << "1;0,0,0" ;
		//server.send is an asynchronous function
		server.send(connection, send_stream, [](const boost::system::error_code& ec) {
			if (ec) {
				cout << "Server: Error sending message. " <<
					//See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
					"Error: " << ec << ", error message: " << ec.message() << endl;
			}
		});
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