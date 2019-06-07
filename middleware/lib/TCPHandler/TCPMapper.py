from utils import BasicMapper


class TCPMapper(BasicMapper):

    def __init__(self, **kwargs):
        super().__init__(load_from_schema=False, **kwargs)

        self._queries.update({
            "get_file_by_pathname": """
                                    SELECT
                                        type, size, order_num
                                    FROM
                                        public.File
                                    WHERE
                                        pathname=%s;
                                    """,
            "get_file_attr_by_pathname": """
                                         SELECT
                                            type, size
                                         FROM
                                            public.File
                                         WHERE
                                            pathname=%s;
                                         """,
            "get_direct_child": """
                                SELECT
                                    pathname
                                FROM
                                    public.File
                                WHERE
                                    file_id IN(
                                    SELECT
                                        unnest(data)
                                    FROM
                                        public.File
                                    WHERE
                                        pathname=%s AND type='d'
                                    )
                                """
        })

    def load_from_file(self, file_path=None):
        if file_path:
            self._load_from_schema(file_path)
        else:
            self._load_from_schema()
