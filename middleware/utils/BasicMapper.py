import psycopg2

from .Config import Config
from .utilities import Singleton

CF = Config()


class BasicMapper(metaclass=Singleton):
    def __init__(self, load_from_schema=False, **kwargs):
        self._queries = {}
        try:
            self.database = kwargs['database'] if kwargs.get('database') else CF.get("Database", "database")
            self.user = kwargs['user'] if kwargs.get('user') else CF.get("Database", "user")
            self.host = kwargs['host'] if kwargs.get('host') else CF.get("Database", "host")
            self.port = kwargs['port'] if kwargs.get('port') else CF.get("Database", "port")
            self.password = kwargs['password'] if kwargs.get('password') else CF.get("Database", "password")
        except Exception as ex:
            raise RuntimeError('Cannot connect to DB: {}'.format(ex))

        if load_from_schema:
            self._connect()

    def _connect(self):
        return psycopg2.connect(database=self.database, user=self.user, host=self.host, password=self.password, port=self.port)

    def _load_from_schema(self, path_name=CF.get("Database", "schema")):
        db = self._connect()
        cursor = db.cursor()
        cursor.execute(open(path_name, "r").read())
        db.commit()
        cursor.close()
        db.close()

    def _dbh(self, query, params=None, last_row_id=False):
        if query in self._queries:
            db = self._connect()
            cursor = db.cursor()
            try:
                if params:
                    cursor.execute(self._queries[query], params)
                else:
                    cursor.execute(self._queries[query])
                db.commit()
            except Exception as ex:
                print('dbh: {} <sql:{}, param:{}>'.format(ex, query, params))
                db.rollback()
                raise ex
            try:
                result = cursor.fetchall()
                lastrowid = cursor.lastrowid
                cursor.close()
                db.close()
                if last_row_id:
                    return result, lastrowid
                return result
            except Exception:
                pass
        else:
            raise Exception("Unknown query")

    def query(self, query, params=None, last_row_id=False):
        if (params is not None) and (not isinstance(params, tuple)):
            params = (params,)
        return self._dbh(query, params, last_row_id=last_row_id)
