/*
    This file is part of RetroSpy Server.

    RetroSpy Server is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef RETROSPYCOMMON_STRINGSERVER_H
#define RETROSPYCOMMON_STRINGSERVER_H

#include <MDK/TemplateServer.h>

/*
	This class is a Server that handle generic
	string packets (like GPSP requests)
*/
class CTemplateStringServer : public CTemplateServer
{
public:
	CTemplateStringServer();
	~CTemplateStringServer();

	/* See CServer::OnRead */
	void OnRead(mdk_socket stream, const char *data, ssize_t size);

	/* See CServer::OnNewConnection */
	virtual bool OnNewConnection(mdk_socket stream, int status) = 0;

	/*
		Function: HandleRequest
		Description: Called when a request is received
		Return: true if you want to keep the connection, otherwise false
		Parameters:
			stream => A pointer to the client
			req => The request
			buf => The content of the request
			size => The length of the content
	*/
	virtual bool HandleRequest(mdk_socket stream, const char *req, const char *buf, int size);
};

#endif
