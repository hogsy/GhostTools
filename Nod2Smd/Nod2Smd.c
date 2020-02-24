/* 
	GhostTools : Nod2Smd
	Copyright (C) 2020 Mark E Sowden 
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef float  Vector;
typedef Vector Vector2[ 2 ];
typedef Vector Vector3[ 3 ];
typedef Vector Vector4[ 4 ];

/* 
40... OR 56
8 bytes 112 = 14 / 10

OR!!!

12 bytes each?

24 verts? Or faces?

128 48

96 / 2


Reading "DFLTshaderCube.nod"...
	version: 10
	numMaterials: 1
	numMeshes: 1
	numVertices: 24
	numTriangles: 144
	Materials
	 DFLTshader

Reading "DummyGun.nod"...
	version: 10
	numMaterials: 1
	numMeshes: 1
	numVertices: 337
	numTriangles: 2048
	Materials
	 BattleMarine_gloss

Reading "cube.nod"...
	version: 10
	numMaterials: 1
	numMeshes: 1
	numVertices: 54
	numTriangles: 144
	Materials
	 initialShadingGroup

1344
*/

typedef struct NodHeader {
	uint32_t version;
	uint8_t  numMaterials;
	uint8_t  numBones;
	uint8_t  u1;
	uint8_t  numMeshes; /* ? */

	char u3[ 32 ]; /* todo */

	uint32_t numVertices;
	char     u5[ 24 ];
	uint32_t numIndices;
	char     u6[ 20 ];
} NodHeader;
static_assert( sizeof( NodHeader ) == 92, "Invalid size for NodHeader!" );

typedef struct NodVertex {
	Vector3 coordinate;
	Vector3 normal;
	Vector2 uv;
} NodVertex;

typedef struct NodTriangle {
	uint16_t vertexIndices[ 3 ];
} NodTriangle;

typedef struct NodBone {
	int8_t blob[ 64 ];
} NodBone;

typedef struct NodMeshSet {
	int8_t bvlah;
};

typedef char NodMaterialName[ 32 ];

#define NOD_VERSION 10

void* AllocOrDie( size_t count, size_t size ) {
	void* pool = calloc( count, size );
	if( pool == NULL ) {
		printf( "Failed to allocate %d bytes, aborting!\n", count * size );
		exit( EXIT_FAILURE );
	}

	return pool;
}

int main( int argc, char** argv ) {
	printf(
		"Nod2Smd Version 1.0.0\n"
		"Copyright (C) 2020 Mark E Sowden <markelswo@gmail.com>\n"
	);

	if( argc < 2 ) {
		printf(
			"Usage:\n"
			" Nod2Smd.exe \"/path/to/nod.nod\" \"/path/to/out.smd\"\n"
		);

		return EXIT_SUCCESS;
	}

	const char* input = argv[ 1 ];
	char output[ 32 ];
	if( argc > 2 ) {
		strcpy( output, argv[ 2 ] );
	} else {
		const char* strPtr = strrchr( input, '/' );
		if( ( strPtr = strrchr( input, '/' ) ) != NULL || ( strPtr = strrchr( input, '\\' ) ) != NULL ) {
			snprintf( output, sizeof( output ), "%s.smd", strPtr );
		} else {
			snprintf( output, sizeof( output ), "%s.smd", input );
		}
	}

	FILE* filePtr = fopen( input, "rb" );
	if( filePtr == NULL ) {
		printf( "Failed to open \"%s\"!\n", input );
		return EXIT_FAILURE;
	}

	printf( "Reading \"%s\"...\n", input );

	NodHeader header;
	if( fread( &header, sizeof( NodHeader ), 1, filePtr ) != 1 ) {
		printf( "Failed to read in Nod header, might not be a valid Nod file!\n" );
		return EXIT_FAILURE;
	}

	/* check the version is valid */
	if( header.version != NOD_VERSION ) {
		printf( "Invalid Nod version (%d != %d)!\n", header.version, NOD_VERSION );
		return EXIT_FAILURE;
	}

#define PrintVar( a, b ) printf( "    " #a ": " #b "\n", header.a )
	PrintVar( version, %d );
	PrintVar( numMaterials, %d );
	PrintVar( numBones, %d );
	PrintVar( u1, % d );
	PrintVar( numMeshes, %d );
	PrintVar( numVertices, %d );
	PrintVar( numIndices, %d );

	fseek( filePtr, 92, SEEK_SET );

	/* now read in all of the materials... */
	NodMaterialName* materialList = AllocOrDie( header.numMaterials, sizeof( NodMaterialName ) );
	if( fread( materialList, sizeof( NodMaterialName ), header.numMaterials, filePtr ) != header.numMaterials ) {
		printf( "Failed to read in all materials!\n" );
		return EXIT_FAILURE;
	}

	printf( "    Materials\n" );
	for( unsigned int i = 0; i < header.numMaterials; ++i ) {
		printf( "     %s\n", materialList[ i ] );
	}

	NodBone* boneList = AllocOrDie( header.numBones, sizeof( NodBone ) );
	if( fread( boneList, sizeof( NodBone ), header.numBones, filePtr ) != header.numBones ) {
		printf( "Failed to read in all bones!\n" );
		return EXIT_FAILURE;
	}

	NodVertex* vertexList = AllocOrDie( header.numVertices, sizeof( NodVertex ) );
	if( fread( vertexList, sizeof( NodVertex ), header.numVertices, filePtr ) != header.numVertices ) {
		printf( "Failed to read in all vertices!\n" );
		return EXIT_FAILURE;
	}

	printf( "    Vertices\n" );
	for( unsigned int i = 0; i < header.numVertices; ++i ) {
		printf( "     %f %f %f\n",
			vertexList[ i ].coordinate[ 0 ],
			vertexList[ i ].coordinate[ 1 ],
			vertexList[ i ].coordinate[ 2 ] );
	}

	unsigned int numTriangles = ( header.numIndices / 3 ); //- 216;
	NodTriangle* triangleList = AllocOrDie( numTriangles, sizeof( NodTriangle ) );
	if( fread( triangleList, sizeof( NodTriangle ), numTriangles, filePtr ) != numTriangles ) {
		printf( "Failed to read in all triangles!\n" );
		return EXIT_FAILURE;
	}

	printf( "Read in %d triangles...\n", numTriangles );

	fclose( filePtr );
	filePtr = NULL;

	/* and now we write our smd... */
	{
		filePtr = fopen( output, "w" );
		fprintf( filePtr,
			"// Generated with Nod2Smd, written by Mark Sowden <markelswo@gmail.com>\n"
			"version 1\n"
			"nodes\n"
		);

		/* todo: handle bones! */
		fprintf( filePtr, "0 \"root\" -1\n" );
		fprintf( filePtr, "end\n" );

		/* triangles... */
		fprintf( filePtr, "triangles\n" );
		for( unsigned int i = 0; i < numTriangles; ++i ) {
			const NodVertex* curVertices[ 3 ] = { NULL, NULL, NULL };
			for( unsigned int j = 0; j < 3; j++ ) {
				if( triangleList[ i ].vertexIndices[ j ] >= header.numVertices ) {
					printf( "Invalid vertex index (%d >= %d)!\n", triangleList[ i ].vertexIndices[ j ], header.numVertices );
					continue;
				}

				curVertices[ j ] = &vertexList[ triangleList[ i ].vertexIndices[ j ] ];
			}

			printf( "Triangle %d, %d, %d\n", triangleList[ i ].vertexIndices[ 0 ], triangleList[ i ].vertexIndices[ 1 ], triangleList[ i ].vertexIndices[ 2 ] );

			if( curVertices[ 0 ] == NULL || curVertices[ 1 ] == NULL || curVertices[ 2 ] == NULL ) {
				continue;
			}

			fprintf( filePtr, "%s\n", materialList[ 0 ] );

			for( unsigned int j = 0; j < 3; ++j ) {
				fprintf( filePtr, "%d %f %f %f %f %f %f %f %f %d %d %f\n",
					/* parent bone */
					0,
					/* pos x y z */
					curVertices[ j ]->coordinate[ 0 ],
					curVertices[ j ]->coordinate[ 1 ],
					curVertices[ j ]->coordinate[ 2 ],
					/* nor x y z */
					curVertices[ j ]->normal[ 0 ],
					curVertices[ j ]->normal[ 1 ],
					curVertices[ j ]->normal[ 2 ],
					/* u v */
					curVertices[ j ]->uv[ 0 ],
					curVertices[ j ]->uv[ 1 ],
					/* links */
					0,
					/* bone id */
					0,
					/* weight */
					0.0f
				);
			}
		}
		fprintf( filePtr, "end\n" );

		fclose( filePtr );

		printf( "Wrote \"%s\"\n", output );
	}

	printf( "Done!\n" );

	return EXIT_SUCCESS;
}