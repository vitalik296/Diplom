from utils import BasicMapper


class HealthMapper(BasicMapper):

    def __init__(self, **kwargs):
        super().__init__(load_from_schema=False, **kwargs)

        self._queries.update({
            "get_node_by_address":  """
                                SELECT
                                    node_id
                                FROM
                                    public.Node
                                WHERE
                                    ip=%s AND tcp_port=%s;
                                    """,
            "insert_new_node": """
                                INSERT INTO
                                    public.Node(ip, tcp_port, udp_port)
                                VALUES
                                    (%s, %s, %s)
                                RETURNING
                                    node_id;
                               """,
            "get_unready_package":  """
                                SELECT
                                    COUNT(*)
                                FROM
                                    public.package
                                WHERE
                                    status=False AND file_id=(SELECT
                                                file_id
                                            FROM
                                                public.package
                                            WHERE 
                                                pack_id=%s)
                                UNION ALL SELECT
                                        file_id
                                    FROM
                                        public.package
                                    WHERE 
                                        pack_id=%s;
                                    """,
            "update_package_status": """
                                UPDATE
                                    public.Package
                                SET
                                    status=%s
                                WHERE
                                    pack_id=%s;
                                     """,
            "update_file_size": """
                                UPDATE
                                    public.File
                                SET
                                    size=size+%s
                                WHERE
                                    file_id=%s;
                                """
        })

    def load_from_file(self, file_path=None):
        if file_path:
            self._load_from_schema(file_path)
        else:
            self._load_from_schema()
