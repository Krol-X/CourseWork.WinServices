struct Address {
	union {
		struct {
			unsigned char a, b, c, d;
		};
		unsigned int value;
	} addr;
	unsigned short port;
};


class Server {
	private:
		bool active;
	public:
		HWND hwnd;
		Server() {
		}
		bool Start(WORD port) {
			active = true;
			return true;
		}
		bool Active() {
			return active;
		}
		void Stop() {
			active = false;
		}
		~Server() {
		}

};

class Client {
	private:

	public:
		HWND hwnd;
		// ...
};
