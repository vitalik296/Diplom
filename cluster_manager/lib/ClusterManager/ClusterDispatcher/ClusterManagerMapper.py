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
                                pathname=%s AND type='d';
                             """,
            "insert_directory": """
                            INSERT INTO
                                public.File(pathname, type)
                            VALUES
                                (%s,'d')
                            RETURNING
                                file_id;
                            """,
            "select_file_id_by_pathname":   """
                            SELECT
                                file_id
                            FROM
                                public.File
                            WHERE
                                pathname=%s;
                                            """,
            "update_file_status_by_file_id": """
                            UPDATE
                                public.File
                            SET
                                status=%s
                            WHERE
                                file_id=%s;
                                      """,
            "insert_package": """
                            INSERT INTO
                                public.Package(node_id, file_id)
                            VALUES
                                (%s, %s)
                            RETURNING
                                pack_id;
                              """,
            "select_file_info": """
                            SELECT
                                order_num, file_id
                            FROM
                                public.File
                            WHERE
                                pathname=%s
                                """,
            "update_package": """
                            UPDATE
                                public.Package
                            SET
                                next_package=%s
                            WHERE
                                pack_id=%s;
                              """,
            "select_parent_id": """
                            SELECT
                                pack_id
                            FROM
                                public.Package
                            WHERE
                                file_id=%s AND next_package IS Null;
                            """,
            "update_file_order_num": """
                            UPDATE
                                public.File
                            SET
                                order_num=%s
                            WHERE
                                pathname=%s;
                                     """,
            "update_file_data": """
                            UPDATE
                                public.File
                            SET
                                data=array_append(data,%s)
                            WHERE
                                pathname=%s AND type='f';
                                """,
            "select_package_by_pathname": """
                            SELECT 
                                node_id, pack_id, next_package
                            FROM
                                public.Package
                            WHERE
                                file_id=(SELECT
                                            file_id
                                         FROM
                                            public.File
                                         WHERE 
                                            pathname=%s);
                                          """,
            "update_file_status": """
                            UPDATE
                                public.File
                            SET
                                status=%s
                            WHERE
                                pathname=%s;
                                  """
        })

    def load_from_file(self, file_path=None):
        if file_path:
            self._load_from_schema(file_path)
        else:
            self._load_from_schema()
