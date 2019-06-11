import sqlite3


from .Config import Config
from .utilities import Singleton

CF = Config()


class BasicMapper(metaclass=Singleton):
    def __init__(self, load_from_schema=False, **kwargs):
        self._queries = {}
        try:
            self.database = kwargs['database'] if kwargs.get('database') else CF.get("Database", "database")
        except Exception as ex:
            raise RuntimeError('Cannot connect to DB: {}'.format(ex))

        if load_from_schema:
            self._connect()

    def _connect(self):
        return sqlite3.connect(self.database)

    def _load_from_schema(self, path_name=CF.get("Database", "schema")):
        db = self._connect()
        cursor = db.cursor()
        # cursor.execute(open(path_name, "r").read())

        with open(path_name) as fp:
            cursor.executescript(fp.read())

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
