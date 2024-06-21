FROM alpine:3.20 as stage-build

RUN apk add --no-cache build-base cmake

WORKDIR /src

ADD . .

RUN cmake . && make

FROM alpine:3.20

COPY --from=stage-build /src/nuketh /nuketh
COPY --from=stage-build /src/nuketh-csv2bal /nuketh-csv2bal

WORKDIR /data

CMD [ "/nuketh", "nuketh.bal", "result.txt" ]