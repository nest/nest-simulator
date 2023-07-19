
Changes in NEST Server
~~~~~~~~~~~~~~~~~~~~~~

* Improve the security in NEST Server. The user can modify the security options in environment variables:
  * Requests require Bearer tokens. By default, the authentication is on (``NEST_SERVER_DISABLE_AUTH=0``).
  * NEST Server takes also custom token (``NEST_SERVER_ACCESS_TOKEN='alongaccesstoken'``), otherwise it generates token.
  * The CORS origins are restricted. By default, the only allowed CORS origin is ``http://localhost``
    (``NEST_SERVER_CORS_ORIGINS=http://localhost``).
  * Only API calls are enabled. By default, the exec call is disabled (``NEST_SERVER_ENABLE_EXEC_CALL=0``).
  * The code execution is restricted. By default, the restriction is activated (``NEST_SERVER_DISABLE_RESTRICTION=0``).