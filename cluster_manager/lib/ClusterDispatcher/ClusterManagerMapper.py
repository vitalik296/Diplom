from utils import BasicMapper


class ClusterManagerMapper(BasicMapper):

    def __init__(self, **kwargs):
        super().__init__(load_from_schema=False, **kwargs)

        self._queries.update({
            "insert_file":  """
                            INSERT INTO
                                public.File(pathname, type)
                            VALUES
                                (%s,'f')
                            RETURNING
                                file_id;
                            """,
            "update_directory_data": """
                            UPDATE
                                public.File
                            SET
                                data=array_append(data,%s)
                            WHERE
                                pathname=%s;
                             """,
            "insert_directory": """
                            INSERT INTO
                                public.File(pathname, type)
                            VALUES
                                (%s,'d');
                            """,
            "select_file_id_by_pathname":   """
                            SELECT
                                file_id
                            FROM
                                public.File
                            WHERE
                                pathname=%s;
                                            """
            # "delete_file_by_id":  """
            #                 DELETE FROM
            #                 public.File
            #                 WHERE
            #                 file_id=?;
            #                 """
        })

    def load_from_file(self, file_path=None):
        if file_path:
            self._load_from_schema(file_path)
        else:
            self._load_from_schema()
