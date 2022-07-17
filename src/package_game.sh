COPY Develop:NEONnoir/neonnoir TO RAM:NEONnoir/neonnoir
COPY Develop:NEONnoir/neonnoir.info TO RAM:NEONnoir/neonnoir.info
COPY Develop:NEONnoir/data TO RAM:NEONnoir/data ALL

lha -a -r -x a RAM:NEONnoir.lha  RAM:NEONnoir

DELETE Develop:NEONnoir/build ALL
MAKEDIR Develop:NEONnoir/build

COPY RAM:NEONnoir.lha TO Develop:NEONnoir/build
