from utils import BasicMapper


class UDPMapper(BasicMapper):

    def __init__(self, **kwargs):
        super().__init__(load_from_schema=False, **kwargs)

        self._queries.update({
            "get_node_address_by_node_id": """
                                    SELECT
                                        ip, udp_port
                                    FROM
                                        public.Node
                                    WHERE
                                        node_id=%s;
                                            """,
            "get_pathname_by_pack_id": """
                                    SELECT
                                        pathname
                                    FROM
                                        public.File
                                    WHERE
                                        file_id = (SELECT
                                                       file_id
                                                   FROM
                                                       public.Package
                                                   WHERE
                                                       pack_id=%s);
                                        """
        })

    def load_from_file(self, file_path=None):
        if file_path:
            self._load_from_schema(file_path)
        else:
            self._load_from_schema()
